#include "../dag.h"
#include <gtest/gtest.h>

class TestNodeFunctorA final : public dag::INodeFunctor<int> {
public:
  int run() override { return 1111; };
};

class TestNodeFunctorB final : public dag::INodeFunctor<double, int> {
public:
  double run(int i) override { return 123.5; };
};

class TestNodeFunctorC final : public dag::INodeFunctor<float, double, int> {
public:
  float run(const double d, const int i) override {
    return static_cast<float>(d) + static_cast<float>(i);
  }
};

class TestNodeFunctorE final : public dag::INodeFunctor<int, double> {
public:
  int run(double d) override { return 777; }
};

class TestNodeFunctorF final : public dag::INodeFunctor<double, double> {
public:
  double run(double d) override { return 771; }
};

class TestNodeFunctorG final : public dag::INodeFunctor<double, int> {
public:
  double run(int i) override { return 770; }
};

TEST(DagTest, CreateGraphAndGetSortedTasks) {
  // Arrange
  const std::array<std::string, 5> names{"TestNodeA", "TestNodeB", "TestNodeC",
                                         "TestNodeD", "TestNodeE"};
  TestNodeFunctorA functorA{};
  TestNodeFunctorB functorB{};
  TestNodeFunctorC functorC{};
  TestNodeFunctorE functorE{};

  // Act
  dag::Node nodeA{&functorA, "TestNodeA"};
  dag::Node nodeB{&functorB, "TestNodeB"};
  nodeB.addDependency<0, int>(&nodeA);
  dag::Node nodeC{&functorC, "TestNodeC"};
  nodeC.addDependency<1, int>(&nodeA);
  nodeC.addDependency<0, double>(&nodeB);
  dag::Node nodeD{&functorB, "TestNodeD"};
  dag::Node nodeE{&functorE, "TestNodeE"};
  nodeE.addDependency<0, double>(&nodeD);

  dag::Dag<5> graph{};

  graph.addNode(&nodeA);
  graph.addNode(&nodeB);
  graph.addNode(&nodeC);
  graph.addNode(&nodeD);
  graph.addNode(&nodeE);

  const auto nodes = graph.getSortedTasks();

  // Assert
  for (int i = 0; i < 5; ++i) {
    auto node = nodes[i];
    EXPECT_EQ(names[i], node->name());
  }
}

TEST(DagTest, DeepDependencyChain) {
  // Arrange
  TestNodeFunctorA functorA{};
  TestNodeFunctorB functorB{};
  TestNodeFunctorC functorC{};
  TestNodeFunctorE functorE{};

  dag::Node nodeA{&functorA, "TestNodeA"};
  dag::Node nodeB{&functorB, "TestNodeB"};
  nodeB.addDependency<0, int>(&nodeA);
  dag::Node nodeC{&functorC, "TestNodeC"};
  nodeC.addDependency<1, int>(&nodeA);
  nodeC.addDependency<0, double>(&nodeB);
  dag::Node nodeD{&functorB, "TestNodeD"};
  dag::Node nodeE{&functorE, "TestNodeE"};
  nodeE.addDependency<0, double>(&nodeD);

  dag::Dag<5> graph{};
  graph.addNode(&nodeA);
  graph.addNode(&nodeB);
  graph.addNode(&nodeC);
  graph.addNode(&nodeD);
  graph.addNode(&nodeE);

  // Act
  const auto nodes = graph.getSortedTasks();

  // Assert
  std::unordered_map<std::string, int> pos;
  for (int i = 0; i < 5; ++i)
    pos[nodes[i]->name()] = i;

  EXPECT_LT(pos["TestNodeA"], pos["TestNodeB"]);
  EXPECT_LT(pos["TestNodeB"], pos["TestNodeC"]);
  EXPECT_LT(pos["TestNodeC"], pos["TestNodeD"]);
  EXPECT_LT(pos["TestNodeD"], pos["TestNodeE"]);
}

TEST(DagTest, WideDAGBranchingAndMerging) {
  // Arrange
  TestNodeFunctorA functorA{};
  TestNodeFunctorB functorB{};
  TestNodeFunctorC functorC{};
  TestNodeFunctorE functorE{};

  dag::Node nodeA{&functorA, "TestNodeA"};
  dag::Node nodeB{&functorB, "TestNodeB"};
  nodeB.addDependency<0, int>(&nodeA);
  dag::Node nodeC{&functorC, "TestNodeC"};
  nodeC.addDependency<1, int>(&nodeA);
  nodeC.addDependency<0, double>(&nodeB);
  dag::Node nodeD{&functorB, "TestNodeD"};
  dag::Node nodeE{&functorE, "TestNodeE"};
  nodeE.addDependency<0, double>(&nodeD);

  dag::Dag<5> graph{};
  graph.addNode(&nodeA);
  graph.addNode(&nodeB);
  graph.addNode(&nodeC);
  graph.addNode(&nodeD);
  graph.addNode(&nodeE);

  // Act
  const auto nodes = graph.getSortedTasks();

  // Assert
  std::unordered_map<std::string, int> pos;
  for (int i = 0; i < 5; ++i)
    pos[nodes[i]->name()] = i;

  EXPECT_LT(pos["TestNodeA"], pos["TestNodeB"]);
  EXPECT_LT(pos["TestNodeA"], pos["TestNodeC"]);
  EXPECT_LT(pos["TestNodeA"], pos["TestNodeD"]);
  EXPECT_LT(pos["TestNodeB"], pos["TestNodeE"]);
  EXPECT_LT(pos["TestNodeC"], pos["TestNodeE"]);
  EXPECT_LT(pos["TestNodeD"], pos["TestNodeE"]);
}

TEST(DagTest, AllNodesDependOnRoot) {
  // Arrange
  TestNodeFunctorA functorA{};
  TestNodeFunctorB functorB{};
  TestNodeFunctorC functorC{};
  TestNodeFunctorE functorE{};

  dag::Node nodeA{&functorA, "TestNodeA"};
  dag::Node nodeB{&functorB, "TestNodeB"};
  nodeB.addDependency<0, int>(&nodeA);
  dag::Node nodeC{&functorC, "TestNodeC"};
  nodeC.addDependency<1, int>(&nodeA);
  nodeC.addDependency<0, double>(&nodeB);
  dag::Node nodeD{&functorB, "TestNodeD"};
  dag::Node nodeE{&functorE, "TestNodeE"};
  nodeE.addDependency<0, double>(&nodeD);

  dag::Dag<5> graph{};
  graph.addNode(&nodeA);
  graph.addNode(&nodeB);
  graph.addNode(&nodeC);
  graph.addNode(&nodeD);
  graph.addNode(&nodeE);

  // Act
  const auto nodes = graph.getSortedTasks();

  // Assert
  std::unordered_map<std::string, int> pos;
  for (int i = 0; i < 5; ++i)
    pos[nodes[i]->name()] = i;

  for (auto name : {"TestNodeB", "TestNodeC", "TestNodeD", "TestNodeE"}) {
    EXPECT_LT(pos["TestNodeA"], pos[name]);
  }
}

TEST(DagTest, DetectsCycle) {
  // Arrange
  TestNodeFunctorE functorE{};
  TestNodeFunctorF functorF{};
  TestNodeFunctorG functorG{};

  dag::Node nodeA{&functorE, "TestNodeA"};
  dag::Node nodeB{&functorG, "TestNodeB"};
  dag::Node nodeC{&functorF, "TestNodeC"};
  nodeC.addDependency<0, double>(&nodeB);
  nodeA.addDependency<0, double>(&nodeC);
  nodeB.addDependency<0, int>(&nodeA);

  dag::Dag<3> graph{};
  graph.addNode(&nodeA);
  graph.addNode(&nodeB);
  graph.addNode(&nodeC);

  // Act & Assert
  EXPECT_DEATH(
      {
        auto tasks = graph.getSortedTasks();
        (void)tasks;
      },
      ".*");
}

TEST(DagTest, RunFunctorOnDependencies) {
  // Arrange
  TestNodeFunctorA functorA{};
  TestNodeFunctorB functorB{};
  TestNodeFunctorC functorC{};

  dag::Node nodeA{&functorA, "TestNodeA"};
  dag::Node nodeB{&functorB, "TestNodeB"};
  nodeB.addDependency<0, int>(&nodeA);
  dag::Node nodeC{&functorC, "TestNodeC"};
  nodeC.addDependency<1, int>(&nodeA);
  nodeC.addDependency<0, double>(&nodeB);

  // Act
  nodeA.run();
  nodeB.run();
  nodeC.run();

  // Assert
  const float *const outputC = nodeC.getOutput();

  EXPECT_EQ(*outputC, 1234.5);
}

TEST(DagTest, RunFunctorOnDependenciesPointers) {
  // Arrange
  TestNodeFunctorA functorA{};
  TestNodeFunctorB functorB{};
  TestNodeFunctorC functorC{};

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

  auto nodes = graph.getSortedTasks();

  // Act
  for (auto *nodePtr : nodes) {
    const auto *threadTaskPtr =
        dynamic_cast<threadPool::IThreadTask *>(nodePtr);

    threadTaskPtr->run();
  }

  // Assert
  const float *const outputC = nodeC.getOutput();

  EXPECT_EQ(*outputC, 1234.5);
}
