#ifndef DAG_H
#define DAG_H

#include "../thread_pool/thread_task.hpp"

#include <atomic>
#include <cassert>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <utility>

namespace dag {

template <typename OUTPUT, typename... DEPS> class INodeFunctor {
public:
  virtual ~INodeFunctor() = default;

  virtual OUTPUT run(DEPS... deps) = 0;
};

template <typename FUNC, size_t... IS>
constexpr void staticForHelper(FUNC f, std::index_sequence<IS...>) {
  (f(std::integral_constant<size_t, IS>{}), ...);
}

template <size_t N, typename FUNC> constexpr void staticFor(FUNC f) {
  staticForHelper(f, std::make_index_sequence<N>{});
}

class INode {
public:
  virtual ~INode() = default;

  virtual size_t size() = 0;
  virtual INode *getDependency(size_t index) = 0;
  virtual bool isVisited() = 0;
  virtual void setVisited() = 0;
  virtual void activate() = 0;
  virtual void deactivate() = 0;
  virtual bool isActive() = 0;
};

template <typename OUTPUT> class NodeDependency : public INode {
public:
  using ReturnType = OUTPUT;

  virtual OUTPUT *getOutput() = 0;
};

template <typename OUTPUT, typename... INPUTS>
class Node final : public NodeDependency<OUTPUT>,
                   public threadPool::IThreadTask {
public:
  static constexpr size_t dependencyCapacity = sizeof...(INPUTS);

  template <std::size_t N>
  using DependencyType = std::tuple_element_t<N, std::tuple<INPUTS...>>;

  explicit Node(INodeFunctor<OUTPUT, INPUTS...> *const functor,
                std::string name)
      : m_name(std::move(name)), m_functor(functor) {

    static_assert(
        std::is_base_of_v<INodeFunctor<OUTPUT, INPUTS...>,
                          std::remove_pointer_t<decltype(m_functor)>>,
        "Functor must inherit from INodeFunctor with matching signature.");
  }
  Node(const Node &other) = delete;
  Node(Node &&other) = delete;

  Node &operator=(const Node &other) = delete;

  // IThreadTask functionality
  void run() const override {
    std::unique_lock lock(m_rwMtx);

    std::tuple<INPUTS...> args{};
    staticFor<dependencyCapacity>([&args, this](auto I) {
      constexpr size_t index = I.value;
      auto *const nodePtr =
          dynamic_cast<NodeDependency<DependencyType<index>> *>(m_deps[index]);

      std::get<index>(args) = *nodePtr->getOutput();
    });

    std::apply(
        [&](const auto &...outputs) { m_output = m_functor->run(outputs...); },
        args);
  }

  // INode functionality
  size_t size() override { return m_depsSize; }

  INode *getDependency(size_t index) override {
    assert(index < dependencyCapacity);
    return m_deps[index];
  }

  std::string name() const override { return m_name; }

  bool isVisited() override { return m_visited; }

  void setVisited() override { m_visited = true; }

  void activate() override { m_active = true; }

  void deactivate() override { m_active = false; }

  bool isActive() override { return m_active; }

  OUTPUT *getOutput() override {
    std::shared_lock lock(m_rwMtx);
    return &m_output;
  }

  // Node functionality

  template <size_t INDEX, typename OUTPUT_TYPE>
  void addDependency(INode *otherNode) {

    static_assert(std::is_same_v<OUTPUT_TYPE, DependencyType<INDEX>>,
                  "Provided type is not a valid dependency");

    if (m_deps[INDEX] == nullptr) {
      ++m_depsSize;
    }
    m_deps[INDEX] = otherNode;
  }

private:
  std::array<INode *, dependencyCapacity> m_deps{};
  size_t m_depsSize{0};
  bool m_visited{false};
  bool m_active{false};
  std::string m_name;
  mutable OUTPUT m_output{}; // TODO: see where to store the output
  INodeFunctor<OUTPUT, INPUTS...> *m_functor{nullptr};
  mutable std::shared_mutex m_rwMtx;
};

template <size_t NODES_SIZE> class Dag {
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

  std::array<threadPool::IThreadTask *, NODES_SIZE> getSortedTasks() {
    if (!isSorted) {
      size_t sortedTasksSize = 0;
      sortedTasksSize = 0;

      for (int nodeIndex = 0; nodeIndex < m_nodesSize; ++nodeIndex) {
        dfs(m_nodes[nodeIndex], m_sortedTasks, sortedTasksSize);
      }
      isSorted = true;
    }

    return m_sortedTasks;
  }

private:
  std::array<INode *, NODES_SIZE> m_nodes;
  std::array<threadPool::IThreadTask *, NODES_SIZE> m_sortedTasks{};
  bool isSorted = false;
  size_t m_nodesSize{0};

  void dfs(INode *node,
           std::array<threadPool::IThreadTask *, NODES_SIZE> &sortedNodes,
           size_t &sortedNodesSize) const {
    assert(!node->isActive());

    if (node->isVisited()) {
      return;
    }

    node->setVisited();
    node->activate();
    if (node->size() > 0) {
      for (int depIndex = 0; depIndex < node->size(); ++depIndex) {
        dfs(node->getDependency(depIndex), sortedNodes, sortedNodesSize);
      }
    }
    node->deactivate();
    sortedNodes[sortedNodesSize] =
        dynamic_cast<threadPool::IThreadTask *>(node);
    sortedNodesSize++;
  }
};

} // namespace dag

#endif // DAG_H
