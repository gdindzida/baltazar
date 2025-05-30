#include "../thread_pool.h"

#include <benchmark/benchmark.h>
#include <chrono>
#include <random>
#include <thread>

constexpr size_t numberOfIterations = 10;
constexpr size_t numberOfTasks = 200;
constexpr size_t baseMilliseconds = 10;
constexpr size_t jitterMilliseconds = 5;
constexpr size_t numberOfThreads = 8;

int wait_random(const int base_ms, const int jitter_ms) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution dist(0, jitter_ms);

  const int delay = base_ms + dist(gen);

  std::this_thread::sleep_for(std::chrono::milliseconds(delay));

  return 0;
}

static void BM_RunSerial(benchmark::State &state) {

  auto myFunc = []() {
    for (int i = 0; i < numberOfTasks; i++) {
      wait_random(baseMilliseconds, jitterMilliseconds);
    }
    return 0;
  };

  for (auto _ : state) {
    benchmark::DoNotOptimize(myFunc());
  }
}
BENCHMARK(BM_RunSerial)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(numberOfIterations);

static void BM_RunParallel(benchmark::State &state) {

  thread_pool::ThreadPool<numberOfThreads, 30> tpool{};

  auto myFunc = [&tpool]() {
    for (int i = 0; i < numberOfTasks; i++) {
      thread_pool::Task task{};
      task.data = nullptr;
      task.func = [](void *) {
        wait_random(baseMilliseconds, jitterMilliseconds);
      };
      while (!tpool.scheduleTask(task)) {
      }
    }

    tpool.waitForAllTasks();

    return 0;
  };

  for (auto _ : state) {
    benchmark::DoNotOptimize(myFunc());
  }
}
BENCHMARK(BM_RunParallel)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(numberOfIterations);

BENCHMARK_MAIN();