#include "../core.hpp"

#include <benchmark/benchmark.h>
#include <chrono>
#include <future>
#include <random>
#include <thread>

constexpr size_t numberOfIterations = 1;
constexpr size_t numberOfLoops = 1000;
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

struct HelperStruct {
  std::atomic<size_t> *_counter;
  size_t _milliseconds;
  size_t _jitterMilliseconds;
};

static void helperTaskFunction(const HelperStruct &context) {
  context._counter->fetch_add(1UL);
  waitRandom(static_cast<int>(context._milliseconds),
             static_cast<int>(context._jitterMilliseconds));
}

class TestNodeFunctorA final : public dag::INodeFunctor<int> {
public:
  static constexpr int _result = 1111;

  explicit TestNodeFunctorA(const HelperStruct &ctx) : m_context(ctx) {}

  int run() override {
    helperTaskFunction(m_context);
    return _result;
  }

private:
  HelperStruct m_context;
};

class TestNodeFunctorB final : public dag::INodeFunctor<double> {
public:
  static constexpr double _result = 123.5;

  explicit TestNodeFunctorB(const HelperStruct &ctx) : m_context(ctx) {}

  double run() override {
    helperTaskFunction(m_context);
    return _result;
  }

private:
  HelperStruct m_context;
};

class TestNodeFunctorC final : public dag::INodeFunctor<float, double, int> {
public:
  explicit TestNodeFunctorC(const HelperStruct &ctx) : m_context(ctx) {}

  float run(const double d, const int i) override {
    helperTaskFunction(m_context);
    return static_cast<float>(d) + static_cast<float>(i);
  }

private:
  HelperStruct m_context;
};

// NOLINTNEXTLINE
static void BM_RunSerial(benchmark::State &state) {

  std::atomic<size_t> testCounter{0};

  TestNodeFunctorA functorA{{&testCounter, 1UL, 3UL}};
  TestNodeFunctorB functorB{{&testCounter, 10UL, 1UL}};
  TestNodeFunctorC functorC{{&testCounter, 1UL, 1UL}};

  dag::Node nodeA{&functorA, "TestNodeA"};
  dag::Node nodeB{&functorB, "TestNodeB"};
  // dag::Node nodeC{&functorC, "TestNodeC"};
  // nodeC.addDependency<1, int>(&nodeA);
  // nodeC.addDependency<0, double>(&nodeB);

  dag::Dag<2> graph{};
  graph.addNode(&nodeA);
  graph.addNode(&nodeB);
  // graph.addNode(&nodeC);

  std::atomic<bool> stopFlag{false};

  for (auto _ : state) {
    core::runGraphSerialNTimes<2>(graph, stopFlag, numberOfLoops);
  }

  assert(testCounter == 2 * numberOfLoops);
}
// BENCHMARK(BM_RunSerial)
//     ->Unit(benchmark::kMillisecond)
//     ->Iterations(numberOfIterations);

// NOLINTNEXTLINE
static void BM_RunParallel(benchmark::State &state) {

  constexpr bool shouldWaitForPrevioius = false;

  std::atomic<size_t> testCounter{0};

  TestNodeFunctorA functorA{{&testCounter, 1UL, 3UL}};
  TestNodeFunctorB functorB{{&testCounter, 10UL, 1UL}};
  TestNodeFunctorC functorC{{&testCounter, 1UL, 1UL}};

  dag::Node nodeA{&functorA, "TestNodeA"};
  dag::Node nodeB{&functorB, "TestNodeB"};
  // dag::Node nodeC{&functorC, "TestNodeC"};
  // nodeC.addDependency<1, int>(&nodeA);
  // nodeC.addDependency<0, double>(&nodeB);

  dag::Dag<2> graph{};
  graph.addNode(&nodeA);
  graph.addNode(&nodeB);
  // graph.addNode(&nodeC);

  std::atomic<bool> stopFlag{false};

  for (auto _ : state) {
    core::runGraphParallelNTimes<2, numberOfThreads, 10>(
        graph, stopFlag, numberOfLoops, shouldWaitForPrevioius, 10);
  }

  assert(testCounter == 2 * numberOfLoops);
}
BENCHMARK(BM_RunParallel)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(numberOfIterations);

BENCHMARK_MAIN();