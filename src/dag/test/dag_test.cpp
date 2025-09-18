#include "../dag.hpp"
#include "gtest/gtest.h"
#include <array>
#include <functional>
#include <gtest/gtest.h>
#include <ostream>
#include <vector>

class TaskA {
public:
  double operator()(int a, float b) {
    return static_cast<double>(a) + static_cast<double>(b);
  }
};

class TaskB {
public:
  TaskB(int a) : m_a(a) {}

  int operator()() { return m_a; }

private:
  int m_a;
};

class TaskC {
public:
  TaskC(float a) : m_a(a) {}

  float operator()() { return m_a; }

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
    return {a.someNum, a.someOtherNum};
  }
};

class TaskE {
public:
  TaskE(int a, int b) : m_a(a), m_b(b) {}

  MyDataType operator()() { return {m_a, m_b}; }

private:
  int m_a;
  int m_b;
};

class TaskF {
public:
  std::string operator()() { return "mystring"; }
};

class TaskG {
public:
  double operator()(double a, std::array<int, 2> b) {
    return a + static_cast<double>(b[0] + b[1]);
  }

private:
};

TEST(DagTest, ConnectFewNodesAndRunThem) {
  // Arrange
  TaskA taskA;
  TaskB taskB{2};
  TaskC taskC{3.f};

  // Act
  dag::Node<2, TaskA> nodeA{taskA, "nodeA"};
  dag::Node<0, TaskB> nodeB{taskB, "nodeB"};
  dag::Node<0, TaskC> nodeC{taskC, "nodeC"};

  nodeA.setDependencyAt<0>(nodeB);
  nodeA.setDependencyAt<1>(nodeC);

  nodeB.run();
  nodeB.setDone();
  nodeC.run();
  nodeC.setDone();
  if (nodeA.isReady()) {
    nodeA.run();
  }

  double retValue = *static_cast<double *>(nodeA.getOutputPtr());

  // Assert
  EXPECT_EQ(retValue, 5.0);
}

TEST(DagTest, ConnectFewNodesAndRunThemButOneDepIsNull) {
  // Arrange
  TaskA taskA;
  TaskB taskB{2};

  // Act & Assert
  dag::Node<2, TaskA> nodeA{taskA, "nodeA"};
  dag::Node<0, TaskB> nodeB{taskB, "nodeB"};

  nodeA.setDependencyAt<0>(nodeB);

  nodeB.run();
  nodeB.setDone();
  EXPECT_DEATH(nodeA.isReady(), ".*");
}

TEST(DagTest, ConnectFewNodesAndRunThemButDepIsNotDone) {
  // Arrange
  TaskA taskA;
  TaskB taskB{2};
  TaskC taskC{3.f};

  // Act & Assert
  dag::Node<2, TaskA> nodeA{taskA, "nodeA"};
  dag::Node<0, TaskB> nodeB{taskB, "nodeB"};
  dag::Node<0, TaskC> nodeC{taskC, "nodeC"};

  nodeA.setDependencyAt<0>(nodeB);
  nodeA.setDependencyAt<1>(nodeC);

  nodeB.run();
  nodeB.setDone();
  nodeC.run();
  EXPECT_DEATH(nodeA.run(), ".*");
}

TEST(DagTest, ConnectFewNodesAndRunThemNonConvertibleTypes) {
  // Arrange
  TaskD taskD;
  TaskE taskE{2, 3};
  TaskF taskF;

  // Act
  dag::Node<2, TaskD> nodeD{taskD, "nodeD"};
  dag::Node<0, TaskE> nodeE{taskE, "nodeE"};
  dag::Node<0, TaskF> nodeF{taskF, "nodeF"};

  nodeD.setDependencyAt<0>(nodeE);
  nodeD.setDependencyAt<1>(nodeF);

  nodeE.run();
  nodeE.setDone();
  nodeF.run();
  nodeF.setDone();
  if (nodeD.isReady()) {
    nodeD.run();
  }

  auto retValue = *static_cast<std::array<int, 2> *>(nodeD.getOutputPtr());
  auto retValueRef = nodeD.getOutputRef();

  // Assert
  EXPECT_EQ(retValue[0], 2);
  EXPECT_EQ(retValue[1], 3);
  EXPECT_EQ(retValue[0], retValueRef[0]);
  EXPECT_EQ(retValue[1], retValueRef[1]);
}

TEST(DagTest, CreateGraphAndGetSortedTasks) {
  // Arrange
  const std::array<std::string, 7> names{"nodeB", "nodeC", "nodeA", "nodeE",
                                         "nodeF", "nodeD", "nodeG"};
  TaskA taskA;
  TaskB taskB{2};
  TaskC taskC{3.f};
  TaskD taskD;
  TaskE taskE{2, 3};
  TaskF taskF;
  TaskG taskG;

  // Act
  dag::Node<2, TaskA> nodeA{taskA, "nodeA"};
  dag::Node<0, TaskB> nodeB{taskB, "nodeB"};
  dag::Node<0, TaskC> nodeC{taskC, "nodeC"};
  nodeA.setDependencyAt<0>(nodeB);
  nodeA.setDependencyAt<1>(nodeC);

  dag::Node<2, TaskD> nodeD{taskD, "nodeD"};
  dag::Node<0, TaskE> nodeE{taskE, "nodeE"};
  dag::Node<0, TaskF> nodeF{taskF, "nodeF"};
  nodeD.setDependencyAt<0>(nodeE);
  nodeD.setDependencyAt<1>(nodeF);

  dag::Node<2, TaskG> nodeG{taskG, "nodeG"};
  nodeG.setDependencyAt<0>(nodeA);
  nodeG.setDependencyAt<1>(nodeD);

  dag::NodeList<7> nodeList;

  nodeList.addNode(&nodeA);
  nodeList.addNode(&nodeB);
  nodeList.addNode(&nodeC);
  nodeList.addNode(&nodeD);
  nodeList.addNode(&nodeE);
  nodeList.addNode(&nodeF);
  nodeList.addNode(&nodeG);

  nodeList.sortNodes();

  // Assert
  for (int i = 0; i < nodeList.getNumberOfNodes(); ++i) {
    auto node = nodeList.getNodeAt(i);
    EXPECT_EQ(names[i], nodeList.getNodeAt(i)->name());
  }
}