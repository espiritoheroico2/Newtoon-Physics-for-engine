// Newtoon Physics - Pool de threads PERSISTENTE com balanceamento dinamico (sem criar threads por quadro).
// ParallelRange divide [0, n) em blocos e distribui via indice atomico; abaixo de `grain` roda no chamador.
#pragma once
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

namespace Newtoon::Core::detail {

// Threads que trabalham em um ParallelRange (inclui o chamador). Sempre >= 1.
inline size_t WorkerThreads() {
  const unsigned hw = std::thread::hardware_concurrency();
  return hw == 0 ? 1 : hw;
}

// Pool persistente singleton: workers dormem em condition_variable quando ociosos (zero CPU)
// e consomem blocos de forma dinamica (work-stealing por fetch_add) durante um ParallelRange.
class PersistentPool {
 public:
  static PersistentPool& Get() {
    static PersistentPool pool;
    return pool;
  }

  size_t ThreadCount() const noexcept { return workers_.size() + 1; }

  template <class F>
  void ParallelFor(size_t n, size_t grain, F&& fn) {
    if (n == 0) return;
    if (grain == 0) throw std::invalid_argument("ParallelRange: grain deve ser > 0");
    if (n <= grain || workers_.empty()) {
      fn(size_t{0}, n);
      return;
    }

    // Evita reentrancia caso um worker chame ParallelFor aninhado.
    std::unique_lock<std::mutex> dispatchLock(dispatchMutex_, std::try_to_lock);
    if (!dispatchLock.owns_lock()) {
      fn(size_t{0}, n);
      return;
    }

    // Divide em ~4 blocos por thread (respeitando `grain`) para balancear carga irregular (ex: narrow-phase).
    const size_t targetChunks = (workers_.size() + 1) * 4;
    const size_t chunk = std::max(grain, (n + targetChunks - 1) / targetChunks);

    std::atomic<size_t> nextBegin{0};
    std::exception_ptr firstError;
    std::mutex errMutex;

    auto workerTask = [&]() {
      try {
        while (true) {
          const size_t b = nextBegin.fetch_add(chunk, std::memory_order_relaxed);
          if (b >= n) break;
          const size_t e = std::min(n, b + chunk);
          fn(b, e);
        }
      } catch (...) {
        std::lock_guard<std::mutex> lk(errMutex);
        if (!firstError) firstError = std::current_exception();
      }
    };

    using TaskFn = decltype(workerTask);
    auto invokeTask = [](void* ctx) { (*static_cast<TaskFn*>(ctx))(); };

    {
      std::lock_guard<std::mutex> lk(queueMutex_);
      taskFn_ = invokeTask;
      taskCtx_ = &workerTask;
      activeWorkers_.store(static_cast<int>(workers_.size()), std::memory_order_release);
      ++generation_;
    }
    cvStart_.notify_all();

    // O chamador tambem participa do trabalho enquanto os workers executam.
    workerTask();

    // Aguarda todos os workers concluirem a geracao atual.
    {
      std::unique_lock<std::mutex> lk(queueMutex_);
      cvDone_.wait(lk, [&]() { return activeWorkers_.load(std::memory_order_acquire) == 0; });
      taskFn_ = nullptr;
      taskCtx_ = nullptr;
    }

    if (firstError) std::rethrow_exception(firstError);
  }

 private:
  PersistentPool() {
    const size_t total = WorkerThreads();
    const size_t numWorkers = total > 1 ? total - 1 : 0;
    workers_.reserve(numWorkers);
    for (size_t i = 0; i < numWorkers; ++i) {
      workers_.emplace_back([this]() { WorkerLoop(); });
    }
  }

  ~PersistentPool() {
    {
      std::lock_guard<std::mutex> lk(queueMutex_);
      stop_ = true;
      ++generation_;
    }
    cvStart_.notify_all();
    for (auto& t : workers_) {
      if (t.joinable()) t.join();
    }
  }

  void WorkerLoop() {
    size_t seenGen = 0;
    while (true) {
      void (*fn)(void*) = nullptr;
      void* ctx = nullptr;
      {
        std::unique_lock<std::mutex> lk(queueMutex_);
        cvStart_.wait(lk, [&]() { return stop_ || generation_ != seenGen; });
        if (stop_) return;
        seenGen = generation_;
        fn = taskFn_;
        ctx = taskCtx_;
      }
      if (fn) fn(ctx);
      if (activeWorkers_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        std::lock_guard<std::mutex> lk(queueMutex_);
        cvDone_.notify_one();
      }
    }
  }

  std::vector<std::thread> workers_;
  std::mutex dispatchMutex_;
  std::mutex queueMutex_;
  std::condition_variable cvStart_;
  std::condition_variable cvDone_;
  void (*taskFn_)(void*) = nullptr;
  void* taskCtx_ = nullptr;
  std::atomic<int> activeWorkers_{0};
  size_t generation_ = 0;
  bool stop_ = false;
};

template <class F>
void ParallelRange(size_t n, size_t grain, F&& fn) {
  PersistentPool::Get().ParallelFor(n, grain, std::forward<F>(fn));
}

} // namespace Newtoon::Core::detail
