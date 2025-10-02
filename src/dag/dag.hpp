#ifndef BALTAZAR_DAG_HPP
#define BALTAZAR_DAG_HPP

#include "../thread_pool/thread_task.hpp"
#include "../utils/function_traits.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <functional>
#include <tuple>
#include <type_traits>
#include <unistd.h>
#include <utility>

namespace baltazar {
namespace dag {

template <size_t NUM_OF_EDGES, typename FUNCTOR> class Node;
template <size_t NUM_OF_NODES> class NodeList;

class INode : public threadPool::IThreadTask {
public:
  virtual ~INode() = default;

  virtual bool isReady() const = 0;
  virtual bool isDone() const = 0;
  virtual void setDone() = 0;
  virtual void reset() = 0;
  virtual void *getOutputPtr() = 0;
  virtual size_t numberOfDeps() = 0;
  virtual INode *getDepAt(size_t index) = 0;
  virtual void setPriority(size_t prio) = 0;
  virtual size_t getPriority() const = 0;
  virtual void setDepth(size_t depth) = 0;
  virtual size_t getDepth() const = 0;

protected:
  virtual bool isActive() = 0;
  virtual bool isVisited() = 0;
  virtual void setVisited() = 0;
  virtual void activate() = 0;
  virtual void deactivate() = 0;
  virtual void resetVisited() = 0;

private:
  template <size_t N, typename F> friend class Node;
  template <size_t N> friend class NodeList;
};

struct Empty {};

template <size_t NUM_OF_DEPS, typename FUNCTOR> class Node : public INode {
public:
  using Traits = utils::FunctionTraits<FUNCTOR>;
  using Output = typename Traits::ReturnType;
  using Args = typename Traits::ArgsTuple;
  using StorageType =
      std::conditional_t<std::is_void_v<Output>, struct Empty, Output>;
  static constexpr size_t argsSize = Traits::ArgsSize;

  Node(const FUNCTOR &f, size_t identifer)
      : m_functor(f), m_identifier(identifer) {
    for (int i = 0; i < NUM_OF_DEPS; i++) {
      m_deps[i] = nullptr;
    }
  }

  Node(FUNCTOR &&f, size_t identifer) : m_functor(f), m_identifier(identifer) {
    for (int i = 0; i < NUM_OF_DEPS; i++) {
      m_deps[i] = nullptr;
    }
  }

  template <size_t I, size_t N, typename F>
  void setDependencyAt(Node<N, F> &otherNode) {
    static_assert((I >= 0) && (I < NUM_OF_DEPS), "Index is out of bounds.");

    using OtherTraits = utils::FunctionTraits<F>;
    using OtherOutput = typename OtherTraits::ReturnType;
    using ArgType = std::tuple_element_t<I, Args>;

    static_assert(std::is_convertible_v<ArgType, OtherOutput>,
                  "Argument type doesn't match output type of edge node.");

    m_deps[I] = &otherNode;
  }

  // std::enable_if_t<!std::is_void_v<Output>, Output &> getOutputRef() {
  //   return m_output;
  // }

  // INode functionality
  bool isReady() const override {
    if (NUM_OF_DEPS == 0) {
      return true;
    }

    if (m_ready) {
      return m_ready;
    }

    m_ready = true;
    for (int i = 0; i < NUM_OF_DEPS; i++) {
      assert(m_deps[i] != nullptr && "One dependency is not set!");
      if (!m_deps[i]->isDone()) {
        m_ready = false;
      }
    }

    return m_ready;
  }

  // INode functionality
  void reset() override { m_ready = false; }

  // INode functionality
  void *getOutputPtr() override {
    if constexpr (std::is_void_v<Output>) {
      return nullptr;
    } else {
      return &m_output;
    }
  }

  // INode functionality
  INode *getDepAt(size_t index) override {
    assert(index < NUM_OF_DEPS && "Index out of bounds!");
    return m_deps[index];
  }

  // INode functionality
  size_t numberOfDeps() override { return NUM_OF_DEPS; }

  // INode functionality
  void setPriority(size_t prio) override { m_prio = prio; }

  // INode functionality
  size_t getPriority() const override { return m_prio; }

  // INode functionality
  bool isDone() const override { return m_done; }

  // INode functionality
  void setDone() override { m_done = true; }

  // INode functionality
  void setDepth(size_t depth) override { m_depth = depth; }

  // INode functionality
  size_t getDepth() const override { return m_depth; }

  // IThreadTask functionality
  void run() const override {
    assert(this->isReady() && "Node is not ready to run!");
    runImpl(std::make_index_sequence<argsSize>{});
  }

  // IThreadTask functionality
  size_t getIdentifier() const override { return m_identifier; }

protected:
  bool isActive() override { return m_active; }

  bool isVisited() override { return m_visited; }

  void setVisited() override { m_visited = true; }

  void activate() override { m_active = true; }

  void deactivate() override { m_active = false; }

  void resetVisited() override { m_visited = false; }

private:
  template <std::size_t... Is> void runImpl(std::index_sequence<Is...>) const {
    if constexpr (!std::is_void_v<Output>) {
      m_output = m_functor(*static_cast<std::tuple_element_t<Is, Args> *>(
          m_deps[Is]->getOutputPtr())...);
    } else {
      m_functor(*static_cast<std::tuple_element_t<Is, Args> *>(
          m_deps[Is]->getOutputPtr())...);
    }
  }

  mutable FUNCTOR m_functor;
  std::array<INode *, NUM_OF_DEPS> m_deps;
  mutable bool m_ready{false};
  mutable StorageType m_output;
  bool m_active{false};
  bool m_visited{false};
  bool m_done{false};
  bool m_scheduled{false};
  size_t m_depth{0};
  size_t m_prio{0};
  size_t m_identifier;
};

enum class SortType {
  Topological,
  Depth,
  Priority,
  DepthOrPriority,
  CustomPriority,
};

template <size_t NUM_OF_NODES> class NodeList {
public:
  NodeList() {}

  void addNode(INode *node) {
    assert(m_size < NUM_OF_NODES && "Index out of bounds!");
    m_nodes[m_size] = node;
    m_size++;
  }

  INode *getNodeAt(size_t index) {
    assert(index < m_size && "Index out of bounds!");
    return m_nodes[index];
  }

  size_t getNumberOfNodes() const { return m_size; }

  void sortNodes(
      SortType sortType = SortType::Topological,
      std::function<bool(const INode *, const INode *)> customCompare =
          [](const INode *, const INode *) { return false; }) {
    std::array<INode *, NUM_OF_NODES> sortedNodes{};
    size_t sortedNodesSize{0};

    for (size_t nodeIndex = 0; nodeIndex < m_size; nodeIndex++) {
      INode *currentNode = m_nodes[nodeIndex];
      dfs(currentNode, sortedNodes, sortedNodesSize);
    }

    if (sortType == SortType::Depth) {
      std::sort(sortedNodes.begin(), sortedNodes.end(),
                [](const INode *a, const INode *b) {
                  return a->getDepth() < b->getDepth();
                });
    }

    if (sortType == SortType::Priority) {
      std::sort(sortedNodes.begin(), sortedNodes.end(),
                [](const INode *a, const INode *b) {
                  return a->getPriority() > b->getPriority();
                });
    }

    if (sortType == SortType::DepthOrPriority) {
      std::sort(sortedNodes.begin(), sortedNodes.end(),
                [](const INode *a, const INode *b) {
                  if (a->getDepth() != b->getDepth()) {
                    return a->getDepth() < b->getDepth();
                  }
                  return a->getPriority() > b->getPriority();
                });
    }

    if (sortType == SortType::CustomPriority) {
      assert(customCompare != nullptr && "Custom priority provided is null!");
      std::sort(sortedNodes.begin(), sortedNodes.end(), customCompare);
    }

    assert(sortedNodesSize == m_size &&
           "Fatal error: Sorted and initial array sizes do not match!");
    for (size_t nodeIndex = 0; nodeIndex < sortedNodesSize; nodeIndex++) {
      m_nodes[nodeIndex] = sortedNodes[nodeIndex];
      m_nodes[nodeIndex]->resetVisited();
    }
  }

private:
  void dfs(INode *node, std::array<INode *, NUM_OF_NODES> &sortedNodes,
           size_t &sortedNodesSize) const {
    assert(!node->isActive() && "Cycle in graph detected!");

    if (node->isVisited()) {
      return;
    }

    node->setVisited();
    node->activate();
    if (node->numberOfDeps() > 0) {
      for (int depIndex = 0; depIndex < node->numberOfDeps(); ++depIndex) {
        auto *depNode = node->getDepAt(depIndex);
        dfs(depNode, sortedNodes, sortedNodesSize);
        node->setDepth(std::max(node->getDepth(), depNode->getDepth() + 1UL));
      }
    }
    node->deactivate();
    sortedNodes[sortedNodesSize] = node;
    sortedNodesSize++;
  }

  std::array<INode *, NUM_OF_NODES> m_nodes;
  size_t m_size{0};
};

} // namespace dag
} // namespace baltazar

#endif // BALTAZAR_DAG_HPP
