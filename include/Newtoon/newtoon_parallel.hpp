// Newtoon Physics - paralelismo basico para os laços da fisica (sem dependencia da engine).
// ParallelRange divide [0, n) em blocos contiguos e roda `fn(begin, end)` em threads.
// Abaixo de `grain` itens roda no proprio thread, sem custo de criacao de threads.
#pragma once
#include <algorithm>
#include <cstddef>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace Newtoon::Core::detail {

// Threads que trabalham em um ParallelRange (inclui o chamador). Sempre >= 1.
inline size_t WorkerThreads() {
  const unsigned hw = std::thread::hardware_concurrency();
  return hw == 0 ? 1 : hw;
}

template <class F>
void ParallelRange(size_t n, size_t grain, F&& fn) {
  if (n == 0) return;
  if (grain == 0) throw std::invalid_argument("ParallelRange: grain deve ser > 0");
  const size_t threads = WorkerThreads();
  if (n <= grain || threads == 1) {
    fn(size_t{0}, n);
    return;
  }
  // Blocos de pelo menos `grain` itens, no maximo um por thread.
  const size_t chunk = std::max(grain, (n + threads - 1) / threads);
  std::vector<std::thread> workers;
  workers.reserve(threads);
  std::exception_ptr firstError;
  std::mutex errM;
  auto run = [&](size_t b, size_t e) {
    try {
      fn(b, e);
    } catch (...) {
      std::lock_guard<std::mutex> l(errM);
      if (!firstError) firstError = std::current_exception();
    }
  };
  size_t b = 0;
  for (; b + chunk < n; b += chunk) workers.emplace_back(run, b, b + chunk);
  run(b, n); // ultimo bloco no chamador
  for (auto& t : workers) t.join();
  if (firstError) std::rethrow_exception(firstError);
}

} // namespace Newtoon::Core::detail
