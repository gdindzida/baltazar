#include "../core.h"
#include <gtest/gtest.h>

static void helperTaskFunction(void *context) {
  if (context != nullptr) {
    auto *testCounter = static_cast<std::atomic<size_t> *>(context);
    ++*testCounter;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(2));
}

class TestNodeFunctorA final : public dag::INodeFunctor<int> {
public:
  constexpr int _result = 1111;
  explicit TestNodeFunctorA(void *ctx) : m_context(ctx) {}

  int run() override {
    helperTaskFunction(m_context);
    return _result;
  }

private:
  void *m_context;
};

class TestNodeFunctorB final : public dag::INodeFunctor<double, int> {
public:
  constexpr double _result = 123.5;
  explicit TestNodeFunctorB(void *ctx) : m_context(ctx) {}
  double run(int i) override {
    helperTaskFunction(m_context);
    return _result;
  }

private:
  void *m_context;
};

class TestNodeFunctorC final : public dag::INodeFunctor<float, double, int> {
public:
  explicit TestNodeFunctorC(void *ctx) : m_context(ctx) {}
  float run(const double d, const int i) override {
    helperTaskFunction(m_context);
    return static_cast<float>(d) + static_cast<float>(i);
  }

private:
  void *m_context;
};

class TestNodeFunctorE final : public dag::INodeFunctor<int, double> {
public:
  constexpr int _result = 777;
  explicit TestNodeFunctorE(void *ctx) : m_context(ctx) {}
  int run(double d) override {
    helperTaskFunction(m_context);
    return _result;
  }

private:
  void *m_context;
};

class TestNodeFunctorF final : public dag::INodeFunctor<double, double> {
public:
  constexpr double _result = 771.0;
  explicit TestNodeFunctorF(void *ctx) : m_context(ctx) {}
  double run(double d) override {
    helperTaskFunction(m_context);
    return _result;
  }

private:
  void *m_context;
};

class TestNodeFunctorG final : public dag::INodeFunctor<double, int> {
public:
  constexpr double _result = 770.0;
  explicit TestNodeFunctorG(void *ctx) : m_context(ctx) {}
  double run(int i) override {
    helperTaskFunction(m_context);
    return _result;
  }

private:
  void *m_context;
};

TEST(ThreadPoolTest, CreateThreadsWithManyTasksAndWaitForAll) {
  // Arrange
  constexpr size_t numThreads = 2;
  constexpr size_t numOfTasks = 300;
  std::atomic<size_t> testCounter{0};

  TestNodeFunctorA functorA{&testCounter};
  TestNodeFunctorB functorB{&testCounter};
  TestNodeFunctorC functorC{&testCounter};

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

  // Assert
  EXPECT_EQ(counter, numOfTasks);
}