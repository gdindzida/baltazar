#define PROFILELOG

#include "../core_parallel.hpp"
#include "../core_serial.hpp"
#include "multithreaded_profiling.hpp"
#include "profiling.hpp"

#include <benchmark/benchmark.h>
#include <chrono>
#include <filesystem>
#include <future>
#include <random>
#include <thread>

namespace baltazar {

constexpr size_t numberOfIterations = 1;
constexpr size_t numberOfLoops = 1000;
constexpr size_t numberOfThreads = 8;

class TaskA {
public:
  double operator()(int a, float b) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return static_cast<double>(a) + static_cast<double>(b);
  }
};

class TaskB {
public:
  TaskB(int a) : m_a(a) {}

  int operator()() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return m_a;
  }

private:
  int m_a;
};

class TaskC {
public:
  TaskC(float a) : m_a(a) {}

  float operator()() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return m_a;
  }

private:
  float m_a;
};

struct MyDataType {
  int someNum;
  int someOtherNum;
};

class TaskD {
public:
  std::array<int, 2> operator()(MyDataType a, std::string s) {
    assert(s == "mystring");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return {a.someNum, a.someOtherNum};
  }
};

class TaskE {
public:
  TaskE(int a, int b) : m_a(a), m_b(b) {}

  MyDataType operator()() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return {m_a, m_b};
  }

private:
  int m_a;
  int m_b;
};

class TaskF {
public:
  std::string operator()() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return "mystring";
  }
};

class TaskG {
public:
  double operator()(double a, std::array<int, 2> b) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    m_sum += a + static_cast<double>(b[0] + b[1]);
    return m_sum;
  }

  void set(double value) { m_sum = value; }

private:
  double m_sum{0.0};
};

namespace fs = std::filesystem;

fs::path getNewLogPath() {
  fs::path logDir = fs::current_path() / "logs";
  if (!fs::exists(logDir)) {
    fs::create_directory(logDir);
  }

  auto now = std::chrono::system_clock::now();
  std::time_t now_c = std::chrono::system_clock::to_time_t(now);

  std::stringstream ss;
  ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d_%H-%M-%S");

  std::string filename = "log_" + ss.str() + ".txt";

  fs::path logFile = logDir / filename;

  return logFile;
}

// NOLINTNEXTLINE
static void BM_RunSerial(benchmark::State &state) {
  std::map<std::string, size_t> indexMap{
      {"nodeA", 1}, {"nodeB", 2}, {"nodeC", 3}, {"nodeD", 4},
      {"nodeE", 5}, {"nodeF", 6}, {"nodeG", 7}};
  dag::NodeList<7> nodeList{};
  TaskA taskA;
  TaskB taskB{2};
  TaskC taskC{3.f};
  TaskD taskD;
  TaskE taskE{2, 6};
  TaskF taskF;
  TaskG taskG;

  static dag::Node<2, TaskA> nodeA{taskA, indexMap["nodeA"]};
  nodeA.setPriority(10);
  static dag::Node<0, TaskB> nodeB{taskB, indexMap["nodeB"]};
  nodeB.setPriority(20);
  static dag::Node<0, TaskC> nodeC{taskC, indexMap["nodeC"]};
  nodeC.setPriority(3);
  nodeA.setDependencyAt<0>(nodeB);
  nodeA.setDependencyAt<1>(nodeC);

  static dag::Node<2, TaskD> nodeD{taskD, indexMap["nodeD"]};
  nodeD.setPriority(6);
  static dag::Node<0, TaskE> nodeE{taskE, indexMap["nodeE"]};
  nodeE.setPriority(7);
  static dag::Node<0, TaskF> nodeF{taskF, indexMap["nodeF"]};
  nodeF.setPriority(9);
  nodeD.setDependencyAt<0>(nodeE);
  nodeD.setDependencyAt<1>(nodeF);

  static dag::Node<2, TaskG> nodeG{taskG, indexMap["nodeG"]};
  taskG.set(0.0);
  nodeG = {taskG, indexMap["nodeG"]};
  nodeG.setPriority(4);
  nodeG.setDependencyAt<0>(nodeA);
  nodeG.setDependencyAt<1>(nodeD);

  nodeList.addNode(&nodeA);
  nodeList.addNode(&nodeB);
  nodeList.addNode(&nodeC);
  nodeList.addNode(&nodeD);
  nodeList.addNode(&nodeE);
  nodeList.addNode(&nodeF);
  nodeList.addNode(&nodeG);

  nodeList.sortNodes(dag::SortType::DepthOrPriority);

  std::atomic<bool> stopFlag{false};

#ifdef PROFILELOG
  fs::path logPath = getNewLogPath();
  std::ofstream out(logPath);
  if (!out) {
    std::cerr << "Failed to create log file: " << logPath << "\n";
    return;
  }

  core::SerialCoreRunner<core::MultiThreadedCoreProfiler<100>> runner{&out,
                                                                      true};
  // core::SerialCoreRunner<core::SingleThreadedCoreProfiler<100>> runner{&out,
  //                                                                      true};
  // core::SerialCoreRunner runner;
#else
  core::SerialCoreRunner runner;
#endif

  for (auto _ : state) {
    runner.runNodeListSerialNTimes(nodeList, stopFlag, numberOfLoops);
  }
}
BENCHMARK(BM_RunSerial)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(numberOfIterations);

// NOLINTNEXTLINE
static void BM_RunParallel(benchmark::State &state) {
  std::map<std::string, size_t> indexMap{
      {"nodeA", 1}, {"nodeB", 2}, {"nodeC", 3}, {"nodeD", 4},
      {"nodeE", 5}, {"nodeF", 6}, {"nodeG", 7}};
  dag::NodeList<7> nodeList{};
  TaskA taskA;
  TaskB taskB{2};
  TaskC taskC{3.f};
  TaskD taskD;
  TaskE taskE{2, 6};
  TaskF taskF;
  TaskG taskG;

  static dag::Node<2, TaskA> nodeA{taskA, indexMap["nodeA"]};
  nodeA.setPriority(10);
  static dag::Node<0, TaskB> nodeB{taskB, indexMap["nodeB"]};
  nodeB.setPriority(20);
  static dag::Node<0, TaskC> nodeC{taskC, indexMap["nodeC"]};
  nodeC.setPriority(3);
  nodeA.setDependencyAt<0>(nodeB);
  nodeA.setDependencyAt<1>(nodeC);

  static dag::Node<2, TaskD> nodeD{taskD, indexMap["nodeD"]};
  nodeD.setPriority(6);
  static dag::Node<0, TaskE> nodeE{taskE, indexMap["nodeE"]};
  nodeE.setPriority(7);
  static dag::Node<0, TaskF> nodeF{taskF, indexMap["nodeF"]};
  nodeF.setPriority(9);
  nodeD.setDependencyAt<0>(nodeE);
  nodeD.setDependencyAt<1>(nodeF);

  static dag::Node<2, TaskG> nodeG{taskG, indexMap["nodeG"]};
  taskG.set(0.0);
  nodeG = {taskG, indexMap["nodeG"]};
  nodeG.setPriority(4);
  nodeG.setDependencyAt<0>(nodeA);
  nodeG.setDependencyAt<1>(nodeD);

  nodeList.addNode(&nodeA);
  nodeList.addNode(&nodeB);
  nodeList.addNode(&nodeC);
  nodeList.addNode(&nodeD);
  nodeList.addNode(&nodeE);
  nodeList.addNode(&nodeF);
  nodeList.addNode(&nodeG);

  nodeList.sortNodes(dag::SortType::DepthOrPriority);

  std::atomic<bool> stopFlag{false};
  threadPool::ThreadPool<numberOfThreads, 10> tPool{};

#ifdef PROFILELOG
  fs::path logPath = getNewLogPath();
  std::ofstream out(logPath);
  if (!out) {
    std::cerr << "Failed to create log file: " << logPath << "\n";
    return;
  }

  core::ParallelCoreRunner<core::MultiThreadedCoreProfiler<100>> runner{&out,
                                                                        true};
  // core::ParallelCoreRunner<core::SingleThreadedCoreProfiler<100>>
  // runner{&out,
  //                                                                        true};
  // core::ParallelCoreRunner runner;
#else
  core::ParallelCoreRunner runner;
#endif

  for (auto _ : state) {
    runner.runNodeListParallelNTimes(nodeList, tPool, stopFlag, numberOfLoops);
  }
}
BENCHMARK(BM_RunParallel)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(numberOfIterations);

} // namespace baltazar

BENCHMARK_MAIN();
