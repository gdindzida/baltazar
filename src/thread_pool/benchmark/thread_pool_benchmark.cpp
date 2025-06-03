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

int waitRandom(const int baseMs, const int jitterMs) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution dist(0, jitterMs);

  const int delay = baseMs + dist(gen);

  std::this_thread::sleep_for(std::chrono::milliseconds(delay));

  return 0;
}

// NOLINTNEXTLINE
static void BM_RunSerial(benchmark::State &state) {

  auto myFunc = []() {
    for (int i = 0; i < numberOfTasks; i++) {
      waitRandom(baseMilliseconds, jitterMilliseconds);
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

// NOLINTNEXTLINE
static void BM_RunParallel(benchmark::State &state) {

  threadPool::ThreadPool<numberOfThreads, 30> tpool{};

  auto myFunc = [&tpool] {
    for (int i = 0; i < numberOfTasks; i++) {
      threadPool::Task task{};
      task._func = [] { waitRandom(baseMilliseconds, jitterMilliseconds); };
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