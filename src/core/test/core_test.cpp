#include "../core.hpp"

#include <future>
#include <gtest/gtest.h>

struct HelperStruct {
  std::atomic<size_t> *_counter;
  size_t _milliseconds;
};

static void helperTaskFunction(const HelperStruct &context) {
  context._counter->fetch_add(1UL);
  std::this_thread::sleep_for(std::chrono::milliseconds(context._milliseconds));
}

class TestNodeFunctorA final : public dag::INodeFunctor<int> {
public:
  static constexpr int _result = 1111;

  explicit TestNodeFunctorA(const HelperStruct &ctx) : m_context(ctx) {}

  int run() override {
    std::cout << "Running task A" << std::endl;
    helperTaskFunction(m_context);
    return _result;
  }

private:
  HelperStruct m_context;
};

class TestNodeFunctorB final : public dag::INodeFunctor<double, int> {
public:
  static constexpr double _result = 123.5;

  explicit TestNodeFunctorB(const HelperStruct &ctx) : m_context(ctx) {}

  double run(int i) override {
    std::cout << "Running task B" << std::endl;
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
    std::cout << "Running task C" << std::endl;
    helperTaskFunction(m_context);
    return static_cast<float>(d) + static_cast<float>(i);
  }

private:
  HelperStruct m_context;
};

TEST(CoreTest, VerifyNodeDependencySynchronisation) {
  // Arrange
  constexpr size_t numThreads = 2;
  std::atomic<size_t> testCounter{0};

  TestNodeFunctorA functorA{{&testCounter, 1UL}};
  TestNodeFunctorB functorB{{&testCounter, 10UL}};
  TestNodeFunctorC functorC{{&testCounter, 1UL}};

  dag::Node nodeA{&functorA, "TestNodeA"};
  dag::Node nodeB{&functorB, "TestNodeB"};
  nodeB.addDependency<0, int>(&nodeA);
  dag::Node nodeC{&functorC, "TestNodeC"};
  nodeC.addDependency<1, int>(&nodeA);
  nodeC.addDependency<0, double>(&nodeB);

  dag::Dag<3> graph{};
  graph.addNode(&nodeA);
  graph.addNode(&nodeB);
  graph.addNode(&nodeC);

  // Act
  std::atomic<bool> shouldStop{false};
  core::runGraphParallelOnce<3, numThreads, 10>(graph, shouldStop);

  // Assert
  EXPECT_EQ(testCounter, 3);

  const float *const outputC = nodeC.getOutput();
  EXPECT_EQ(*outputC, 1234.5);
}

TEST(CoreTest, VerifyNodeDependencySynchronisationNTimesAndWaitForPrevious) {
  // Arrange
  constexpr size_t numberOfIterations = 100;
  constexpr bool shouldWaitForPrevious = true;
  constexpr size_t numThreads = 2;
  std::atomic<size_t> testCounter{0};

  TestNodeFunctorA functorA{{&testCounter, 1UL}};
  TestNodeFunctorB functorB{{&testCounter, 1UL}};
  TestNodeFunctorC functorC{{&testCounter, 1UL}};

  dag::Node nodeA{&functorA, "TestNodeA"};
  dag::Node nodeB{&functorB, "TestNodeB"};
  nodeB.addDependency<0, int>(&nodeA);
  dag::Node nodeC{&functorC, "TestNodeC"};
  nodeC.addDependency<1, int>(&nodeA);
  nodeC.addDependency<0, double>(&nodeB);

  dag::Dag<3> graph{};
  graph.addNode(&nodeA);
  graph.addNode(&nodeB);
  graph.addNode(&nodeC);

  // Act
  std::atomic<bool> shouldStop{false};
  core::runGraphParallelNTimes<3, numThreads, 10>(
      graph, shouldStop, numberOfIterations, shouldWaitForPrevious, 1);

  // Assert
  EXPECT_EQ(testCounter, 3 * numberOfIterations);

  const float *const outputC = nodeC.getOutput();
  EXPECT_EQ(*outputC, 1234.5);
}

TEST(CoreTest,
     VerifyNodeDependencySynchronisationNTimesAndDontWaitForPrevious) {
  // Arrange
  constexpr size_t numberOfIterations = 100;
  constexpr bool shouldWaitForPrevious = false;
  constexpr size_t numThreads = 2;
  std::atomic<size_t> testCounter{0};

  TestNodeFunctorA functorA{{&testCounter, 1UL}};
  TestNodeFunctorB functorB{{&testCounter, 1UL}};
  TestNodeFunctorC functorC{{&testCounter, 1UL}};

  dag::Node nodeA{&functorA, "TestNodeA"};
  dag::Node nodeB{&functorB, "TestNodeB"};
  nodeB.addDependency<0, int>(&nodeA);
  dag::Node nodeC{&functorC, "TestNodeC"};
  nodeC.addDependency<1, int>(&nodeA);
  nodeC.addDependency<0, double>(&nodeB);

  dag::Dag<3> graph{};
  graph.addNode(&nodeA);
  graph.addNode(&nodeB);
  graph.addNode(&nodeC);

  // Act
  std::atomic<bool> shouldStop{false};
  core::runGraphParallelNTimes<3, numThreads, 10>(
      graph, shouldStop, numberOfIterations, shouldWaitForPrevious, 10);

  // Assert
  EXPECT_EQ(testCounter, 3 * numberOfIterations);

  const float *const outputC = nodeC.getOutput();
  EXPECT_EQ(*outputC, 1234.5);
}

TEST(
    CoreTest,
    RunParallelLoopAndWaitForPreviousIterationAndStopItToVerifyReasonableNumberOfTasksDone) {
  // Arrange
  constexpr bool shouldWaitForPrevious = true;
  constexpr size_t numThreads = 2;
  std::atomic<size_t> testCounter{0};

  TestNodeFunctorA functorA{{&testCounter, 1UL}};
  TestNodeFunctorB functorB{{&testCounter, 1UL}};
  TestNodeFunctorC functorC{{&testCounter, 1UL}};

  dag::Node nodeA{&functorA, "TestNodeA"};
  dag::Node nodeB{&functorB, "TestNodeB"};
  nodeB.addDependency<0, int>(&nodeA);
  dag::Node nodeC{&functorC, "TestNodeC"};
  nodeC.addDependency<1, int>(&nodeA);
  nodeC.addDependency<0, double>(&nodeB);

  dag::Dag<3> graph{};
  graph.addNode(&nodeA);
  graph.addNode(&nodeB);
  graph.addNode(&nodeC);

  // Act
  std::atomic<bool> shouldStop{false};

  std::future<void> ftr = std::async(
      std::launch::async,
      [](const size_t millis, std::atomic<bool> &stop) {
        std::this_thread::sleep_for(std::chrono::milliseconds(millis));
        stop.store(true);
      },
      15UL, std::ref(shouldStop));

  core::runGraphParallelLoop<3, numThreads, 10>(graph, shouldStop,
                                                shouldWaitForPrevious, 10);

  ftr.wait();

  // Assert
  EXPECT_GE(testCounter, 20);

  const float *const outputC = nodeC.getOutput();
  EXPECT_EQ(*outputC, 1234.5);
}

TEST(
    CoreTest,
    RunParallelLoopAndDontWaitForPreviousIterationAndStopItToVerifyReasonableNumberOfTasksDone) {
  // Arrange
  constexpr bool shouldWaitForPrevious = false;
  constexpr size_t numThreads = 2;
  std::atomic<size_t> testCounter{0};

  TestNodeFunctorA functorA{{&testCounter, 1UL}};
  TestNodeFunctorB functorB{{&testCounter, 1UL}};
  TestNodeFunctorC functorC{{&testCounter, 1UL}};

  dag::Node nodeA{&functorA, "TestNodeA"};
  dag::Node nodeB{&functorB, "TestNodeB"};
  nodeB.addDependency<0, int>(&nodeA);
  dag::Node nodeC{&functorC, "TestNodeC"};
  nodeC.addDependency<1, int>(&nodeA);
  nodeC.addDependency<0, double>(&nodeB);

  dag::Dag<3> graph{};
  graph.addNode(&nodeA);
  graph.addNode(&nodeB);
  graph.addNode(&nodeC);

  // Act
  std::atomic<bool> shouldStop{false};

  std::future<void> ftr = std::async(
      std::launch::async,
      [](const size_t millis, std::atomic<bool> &stop) {
        std::this_thread::sleep_for(std::chrono::milliseconds(millis));
        stop.store(true);
      },
      15UL, std::ref(shouldStop));

  core::runGraphParallelLoop<3, 2, 10>(graph, shouldStop, shouldWaitForPrevious,
                                       10);

  ftr.wait();

  // Assert
  EXPECT_GE(testCounter, 20);

  const float *const outputC = nodeC.getOutput();
  EXPECT_EQ(*outputC, 1234.5);
}
