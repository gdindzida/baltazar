#include "../dag.h"
#include <gtest/gtest.h>

TEST(DagTest, CreateGraphAndGetSortedTasks) {
  // Arrange
  auto work = [] {};

  std::array<std::string, 5> names{"nodeA", "nodeB", "nodeC", "nodeD", "nodeE"};

  // Act
  dag::Node<0> nodeA{{work, "nodeA"}};
  dag::Node<1> nodeB{{work, "nodeB"}};
  nodeB.addDependency(&nodeA);
  dag::Node<2> nodeC{{work, "nodeC"}};
  nodeC.addDependency(&nodeA);
  nodeC.addDependency(&nodeB);
  dag::Node<0> nodeD{{work, "nodeD"}};
  dag::Node<1> nodeE{{work, "nodeE"}};
  nodeE.addDependency(&nodeD);

  dag::Dag<5> graph{};

  graph.addNode(&nodeA);
  graph.addNode(&nodeB);
  graph.addNode(&nodeC);
  graph.addNode(&nodeD);
  graph.addNode(&nodeE);

  auto tasks = graph.getSortedTasks();

  // Assert
  for (int i = 0; i < 5; ++i) {
    const threadPool::Task &task = tasks[i];
    EXPECT_EQ(names[i], task._name);
  }
}

TEST(DagTest, DeepDependencyChain) {
  // Arrange
  auto work = [] {};

  dag::Node<0> nodeA{{work, "A"}};
  dag::Node<1> nodeB{{work, "B"}};
  nodeB.addDependency(&nodeA);
  dag::Node<1> nodeC{{work, "C"}};
  nodeC.addDependency(&nodeB);
  dag::Node<1> nodeD{{work, "D"}};
  nodeD.addDependency(&nodeC);
  dag::Node<1> nodeE{{work, "E"}};
  nodeE.addDependency(&nodeD);

  dag::Dag<5> graph{};
  graph.addNode(&nodeA);
  graph.addNode(&nodeB);
  graph.addNode(&nodeC);
  graph.addNode(&nodeD);
  graph.addNode(&nodeE);

  // Act
  auto tasks = graph.getSortedTasks();

  // Assert
  std::unordered_map<std::string, int> pos;
  for (int i = 0; i < 5; ++i)
    pos[tasks[i]._name] = i;

  EXPECT_LT(pos["A"], pos["B"]);
  EXPECT_LT(pos["B"], pos["C"]);
  EXPECT_LT(pos["C"], pos["D"]);
  EXPECT_LT(pos["D"], pos["E"]);
}

TEST(DagTest, WideDAGBranchingAndMerging) {
  // Arrange
  auto work = [] {};

  dag::Node<0> nodeA{{work, "A"}};
  dag::Node<1> nodeB{{work, "B"}};
  nodeB.addDependency(&nodeA);
  dag::Node<1> nodeC{{work, "C"}};
  nodeC.addDependency(&nodeA);
  dag::Node<1> nodeD{{work, "D"}};
  nodeD.addDependency(&nodeA);
  dag::Node<3> nodeE{{work, "E"}};
  nodeE.addDependency(&nodeB);
  nodeE.addDependency(&nodeC);
  nodeE.addDependency(&nodeD);

  dag::Dag<5> graph{};
  graph.addNode(&nodeA);
  graph.addNode(&nodeB);
  graph.addNode(&nodeC);
  graph.addNode(&nodeD);
  graph.addNode(&nodeE);

  // Act
  auto tasks = graph.getSortedTasks();

  // Assert
  std::unordered_map<std::string, int> pos;
  for (int i = 0; i < 5; ++i)
    pos[tasks[i]._name] = i;

  EXPECT_LT(pos["A"], pos["B"]);
  EXPECT_LT(pos["A"], pos["C"]);
  EXPECT_LT(pos["A"], pos["D"]);
  EXPECT_LT(pos["B"], pos["E"]);
  EXPECT_LT(pos["C"], pos["E"]);
  EXPECT_LT(pos["D"], pos["E"]);
}

TEST(DagTest, AllNodesDependOnRoot) {
  // Arrange
  auto work = [] {};

  dag::Node<0> nodeA{{work, "A"}}; // Root
  dag::Node<1> nodeB{{work, "B"}};
  nodeB.addDependency(&nodeA);
  dag::Node<1> nodeC{{work, "C"}};
  nodeC.addDependency(&nodeA);
  dag::Node<1> nodeD{{work, "D"}};
  nodeD.addDependency(&nodeA);
  dag::Node<1> nodeE{{work, "E"}};
  nodeE.addDependency(&nodeA);

  dag::Dag<5> graph{};
  graph.addNode(&nodeA);
  graph.addNode(&nodeB);
  graph.addNode(&nodeC);
  graph.addNode(&nodeD);
  graph.addNode(&nodeE);

  // Act
  auto tasks = graph.getSortedTasks();

  // Assert
  std::unordered_map<std::string, int> pos;
  for (int i = 0; i < 5; ++i)
    pos[tasks[i]._name] = i;

  for (auto name : {"B", "C", "D", "E"}) {
    EXPECT_LT(pos["A"], pos[name]);
  }
}

TEST(DagTest, DetectsCycle) {
  // Arrange
  auto work = [] {};

  dag::Node<1> nodeA{{work, "A"}};
  dag::Node<1> nodeB{{work, "B"}};
  dag::Node<1> nodeC{{work, "C"}};

  nodeA.addDependency(&nodeC);
  nodeB.addDependency(&nodeA);
  nodeC.addDependency(&nodeB); // Cycle A -> B -> C -> A

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
