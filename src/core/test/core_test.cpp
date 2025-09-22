#include "../core.hpp"

#include <atomic>
#include <gtest/gtest.h>
#include <thread>

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
    m_sum += a + static_cast<double>(b[0] + b[1]);
    return m_sum;
  }

  void set(double value) { m_sum = value; }

private:
  double m_sum{0.0};
};

class CoreTest : public ::testing::Test {

protected:
  void SetUp() override { arrangeTest(); }

  void TearDown() override {}

  dag::NodeList<7> &getNodes() { return m_nodeList; }

  std::array<size_t, 7> &getNames() { return m_names; }

  void arrangeTest() {
    const std::array<size_t, 7> m_names{
        m_indexMap["nodeB"], m_indexMap["nodeF"], m_indexMap["nodeE"],
        m_indexMap["nodeC"], m_indexMap["nodeA"], m_indexMap["nodeD"],
        m_indexMap["nodeG"]};

    static dag::Node<2, TaskA> nodeA{m_taskA, m_indexMap["nodeA"]};
    nodeA.setPriority(10);
    static dag::Node<0, TaskB> nodeB{m_taskB, m_indexMap["nodeB"]};
    nodeB.setPriority(20);
    static dag::Node<0, TaskC> nodeC{m_taskC, m_indexMap["nodeC"]};
    nodeC.setPriority(3);
    nodeA.setDependencyAt<0>(nodeB);
    nodeA.setDependencyAt<1>(nodeC);

    static dag::Node<2, TaskD> nodeD{m_taskD, m_indexMap["nodeD"]};
    nodeD.setPriority(6);
    static dag::Node<0, TaskE> nodeE{m_taskE, m_indexMap["nodeE"]};
    nodeE.setPriority(7);
    static dag::Node<0, TaskF> nodeF{m_taskF, m_indexMap["nodeF"]};
    nodeF.setPriority(9);
    nodeD.setDependencyAt<0>(nodeE);
    nodeD.setDependencyAt<1>(nodeF);

    static dag::Node<2, TaskG> nodeG{m_taskG, m_indexMap["nodeG"]};
    m_taskG.set(0.0);
    nodeG = {m_taskG, m_indexMap["nodeG"]};
    nodeG.setPriority(4);
    nodeG.setDependencyAt<0>(nodeA);
    nodeG.setDependencyAt<1>(nodeD);

    m_nodeList.addNode(&nodeA);
    m_nodeList.addNode(&nodeB);
    m_nodeList.addNode(&nodeC);
    m_nodeList.addNode(&nodeD);
    m_nodeList.addNode(&nodeE);
    m_nodeList.addNode(&nodeF);
    m_nodeList.addNode(&nodeG);

    m_nodeList.sortNodes(dag::SortType::DepthOrPriority);
  }

  std::map<std::string, size_t> m_indexMap{
      {"nodeA", 1}, {"nodeB", 2}, {"nodeC", 3}, {"nodeD", 4},
      {"nodeE", 5}, {"nodeF", 6}, {"nodeG", 7}};
  dag::NodeList<7> m_nodeList{};
  std::array<size_t, 7> m_names{};
  TaskA m_taskA;
  TaskB m_taskB{2};
  TaskC m_taskC{3.f};
  TaskD m_taskD;
  TaskE m_taskE{2, 6};
  TaskF m_taskF;
  TaskG m_taskG;
};

TEST_F(CoreTest, RunSerialOnce) {
  // Arrange
  std::atomic<bool> stopFlag;

  // Act
  core::runNodeListSerialOnce(this->getNodes(), stopFlag);

  double retValue =
      *static_cast<double *>(this->getNodes().getNodeAt(6)->getOutputPtr());

  // Assert
  EXPECT_EQ(retValue, 13.0);
}

TEST_F(CoreTest, RunSerialNTimes) {
  // Arrange
  std::atomic<bool> stopFlag;
  constexpr size_t n = 16;

  // Act
  core::runNodeListSerialNTimes(this->getNodes(), stopFlag, n);

  double retValue =
      *static_cast<double *>(this->getNodes().getNodeAt(6)->getOutputPtr());

  // Assert
  EXPECT_EQ(retValue, n * 13.0);
}

TEST_F(CoreTest, RunSrialeInALoop) {
  // Arrange
  std::atomic<bool> stopFlag{false};

  // Act
  std::thread t(core::runNodeListSerialLoop<7>, std::ref(this->getNodes()),
                std::ref(stopFlag));

  std::this_thread::sleep_for(std::chrono::milliseconds(2));
  stopFlag = true;

  // Assert
  t.join();
  EXPECT_TRUE(true);
}

TEST_F(CoreTest, RunParallelOnce) {
  // Arrange
  std::atomic<bool> stopFlag;
  threadPool::ThreadPool<2, 10> tPoll{};

  // Act
  core::runNodeListParallelOnce(this->getNodes(), tPoll, stopFlag);

  double retValue =
      *static_cast<double *>(this->getNodes().getNodeAt(6)->getOutputPtr());

  // Assert
  EXPECT_EQ(retValue, 13.0);
}

TEST_F(CoreTest, RunParallelNTimes) {
  // Arrange
  std::atomic<bool> stopFlag;
  constexpr size_t n = 16;
  threadPool::ThreadPool<2, 10> tPoll{};

  // Act
  core::runNodeListParallelNTimes(this->getNodes(), tPoll, stopFlag, n);

  double retValue =
      *static_cast<double *>(this->getNodes().getNodeAt(6)->getOutputPtr());

  // Assert
  EXPECT_EQ(retValue, n * 13.0);
}

TEST_F(CoreTest, RunParallelInALoop) {
  // Arrange
  std::atomic<bool> stopFlag{false};
  threadPool::ThreadPool<2, 10> tPoll{};

  // Act
  std::thread t(core::runNodeListParallelLoop<7, 2, 10>,
                std::ref(this->getNodes()), std::ref(tPoll),
                std::ref(stopFlag));

  std::this_thread::sleep_for(std::chrono::milliseconds(2));
  stopFlag = true;

  // Assert
  t.join();
  EXPECT_TRUE(true);
}