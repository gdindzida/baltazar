#ifndef DAG_H
#define DAG_H

#include "../thread_pool/thread_task.h"

#include <cassert>
#include <string>
#include <utility>

namespace dag {

struct INode {
  virtual ~INode() = default;

  virtual constexpr size_t size() = 0;
  virtual void addDependency(INode *dependency) = 0;
  virtual INode *getDependency(size_t index) = 0;
  virtual std::string name() = 0;
  virtual threadPool::Task task() = 0;
  virtual bool isVisited() = 0;
  virtual void setVisited() = 0;
  virtual void activate() = 0;
  virtual void dectivate() = 0;
  virtual bool isActive() = 0;
};

template <size_t DEPS_CAPACITY> class Node final : public INode {
  std::array<INode *, DEPS_CAPACITY> m_deps;
  size_t m_depsSize{0};
  threadPool::Task m_task;
  bool m_visited{false};
  bool m_active{false};

public:
  explicit Node(threadPool::Task task) : m_task(std::move(task)) {}
  Node(const Node &other) = delete;
  Node(Node &&other) = delete;

  Node &operator=(const Node &other) = delete;

  constexpr size_t size() override { return m_depsSize; }

  void addDependency(INode *dependency) override {
    assert(m_depsSize < DEPS_CAPACITY);

    m_deps[m_depsSize] = dependency;
    m_depsSize++;
  }

  INode *getDependency(size_t index) override {
    assert(index < DEPS_CAPACITY);
    return m_deps[index];
  }

  std::string name() override { return m_task._name; }

  threadPool::Task task() override { return m_task; }

  bool isVisited() override { return m_visited; }

  void setVisited() override { m_visited = true; }

  void activate() override { m_active = true; }

  void dectivate() override { m_active = false; }

  bool isActive() override { return m_active; }
};

template <size_t NODES_SIZE> class Dag {
  std::array<INode *, NODES_SIZE> m_nodes;
  size_t m_nodesSize{0};

  void dfs(INode *node, std::array<threadPool::Task, NODES_SIZE> &sortedTasks,
           size_t &sortedTasksSize) const {
    assert(!node->isActive());

    if (node->isVisited()) {
      return;
    }

    node->setVisited();
    node->activate();
    if (node->size() > 0) {
      for (int depIndex = 0; depIndex < node->size(); ++depIndex) {
        dfs(node->getDependency(depIndex), sortedTasks, sortedTasksSize);
      }
    }
    node->dectivate();
    sortedTasks[sortedTasksSize] = node->task();
    sortedTasksSize++;
  }

public:
  Dag() = default;
  Dag(const Dag &other) = delete;
  Dag(Dag &&other) = delete;

  Dag &operator=(const Dag &other) = delete;

  void addNode(INode *dependency) {
    assert(m_nodesSize < NODES_SIZE);

    m_nodes[m_nodesSize] = dependency;
    m_nodesSize++;
  }

  std::array<threadPool::Task, NODES_SIZE> getSortedTasks() const {
    std::array<threadPool::Task, NODES_SIZE> sortedTasks{};
    size_t sortedTasksSize = 0;
    sortedTasksSize = 0;

    for (int nodeIndex = 0; nodeIndex < m_nodesSize; ++nodeIndex) {
      dfs(m_nodes[nodeIndex], sortedTasks, sortedTasksSize);
    }

    return sortedTasks;
  }
};

} // namespace dag

#endif // DAG_H
