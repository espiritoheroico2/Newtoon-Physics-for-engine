#pragma once
#include <vector>
#include <thread>
#include <future>
#include <functional>

namespace Newtoon {

    class JobSystem {
    public:
        // Executa um loop 'for' gigante dividindo-o entre todos os núcleos da CPU
        template<typename T>
        static void ParallelFor(std::vector<T>& data, std::function<void(T&)> task) {
            const size_t numThreads = std::thread::hardware_concurrency();
            const size_t chunkSize = data.size() / numThreads;
            
            std::vector<std::future<void>> futures;

            for (size_t i = 0; i < numThreads; ++i) {
                size_t startIdx = i * chunkSize;
                size_t endIdx = (i == numThreads - 1) ? data.size() : startIdx + chunkSize;

                futures.push_back(std::async(std::launch::async, [&data, startIdx, endIdx, task]() {
                    for (size_t j = startIdx; j < endIdx; ++j) {
                        task(data[j]);
                    }
                }));
            }

            // Espera todas as threads terminarem antes de continuar o frame
            for (auto& f : futures) {
                f.get();
            }
        }
    };
}