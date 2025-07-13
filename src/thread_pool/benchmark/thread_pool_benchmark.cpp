#include "../thread_pool.hpp"

#include <benchmark/benchmark.h>
#include <chrono>
#include <random>
#include <thread>

constexpr size_t numberOfIterations = 10;
constexpr size_t numberOfTasks = 200;
constexpr size_t baseMilliseconds = 10;
constexpr size_t jitterMilliseconds = 5;
constexpr size_t numberOfThreads = 8;

struct BenchmarkData {
  int _baseMilliseconds;
  int _jitterMilliseconds;
};

int waitRandom(const int baseMs, const int jitterMs) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution dist(0, jitterMs);

  const int delay = baseMs + dist(gen);

  std::this_thread::sleep_for(std::chrono::milliseconds(delay));

  return 0;
}

class TestThreadTask final : public threadPool::IThreadTask {
public:
  explicit TestThreadTask(void *ctx) : m_context(ctx) {};

  // NOLINTNEXTLINE
  ~TestThreadTask() override {};

  void run() const override { helperTaskFunction(m_context); }
  std::string name() const override { return "TestThreadTask"; }

private:
  static void helperTaskFunction(void *context) {
    if (context == nullptr) {
      return;
    }
    const auto *const benchmarkData = static_cast<BenchmarkData *>(context);
    waitRandom(benchmarkData->_baseMilliseconds,
               benchmarkData->_jitterMilliseconds);
  }

  void *m_context;
};

// NOLINTNEXTLINE
static void BM_RunSerial(benchmark::State &state) {

  auto myFunc = [] {
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

  threadPool::ThreadPool<numberOfThreads, 0, 30> tpool{};

  auto myFunc = [&tpool] {
    BenchmarkData data{baseMilliseconds, jitterMilliseconds};
    TestThreadTask task{&data};

    for (int i = 0; i < numberOfTasks; i++) {
      while (!tpool.scheduleTask(&task)) {
      }
    }
    std::atomic stop{false};
    tpool.waitForAllTasks(stop);

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