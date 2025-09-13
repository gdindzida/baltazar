#ifndef DAG_H
#define DAG_H

#include "../thread_pool/thread_task.hpp"
#include "../utils/function_traits.hpp"

#include <array>
#include <cassert>
#include <string>
#include <tuple>
#include <type_traits>
#include <unistd.h>
#include <utility>

namespace dag {

template <size_t NUM_OF_EDGES, typename FUNCTOR> class Node;

class INode {
public:
  virtual ~INode() = default;

  virtual bool isReady() const = 0;
  virtual bool isDone() const = 0;
  virtual void setDone() = 0;
  virtual void reset() = 0;
  virtual void *getOutput() = 0;

private:
  template <size_t N, typename F> friend class Node;
};

template <size_t NUM_OF_DEPS, typename FUNCTOR>
class Node : public INode, public threadPool::IThreadTask {
public:
  using Traits = utils::FunctionTraits<FUNCTOR>;
  using Output = typename Traits::ReturnType;
  using Args = typename Traits::ArgsTuple;
  static constexpr size_t argsSize = Traits::ArgsSize;

  Node(FUNCTOR &f, std::string name) : m_functor(f), m_name(name) {
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
      assert(m_deps[i] != nullptr);
      if (!m_deps[i]->isDone()) {
        m_ready = false;
      }
    }

    return m_ready;
  }

  // INode functionality
  bool isDone() const override { return m_done; }

  // INode functionality
  void setDone() override { m_done = true; }

  // INode functionality
  void reset() override {
    m_ready = false;
    m_done = false;
  }

  // INode functionality
  void *getOutput() override { return static_cast<void *>(&m_output); }

  // IThreadTask functionality
  void run() const override {
    assert(this->isReady());
    runImpl(std::make_index_sequence<argsSize>{});
  }

  // IThreadTask functionality
  std::string name() const override { return m_name; }

  // IThreadTask functionality
  bool shouldSyncWhenDone() const override { return true; }

private:
  template <std::size_t... Is> void runImpl(std::index_sequence<Is...>) const {
    m_output = m_functor(*static_cast<std::tuple_element_t<Is, Args> *>(
        m_deps[Is]->getOutput())...);
  }

  mutable FUNCTOR m_functor;
  std::array<INode *, NUM_OF_DEPS> m_deps;
  mutable bool m_ready{false};
  bool m_done{false};
  mutable Output m_output;
  std::string m_name;
};

} // namespace dag

#endif // DAG_H
