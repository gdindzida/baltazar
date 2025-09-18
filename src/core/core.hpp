#ifndef CORE_H
#define CORE_H
#include "../dag/dag.hpp"
#include "../thread_pool/thread_pool.hpp"
#include <atomic>

namespace core {

template <size_t NUM_OF_NODES>
void runNodeListSerialOnce(dag::NodeList<NUM_OF_NODES> nodes,
                           std::atomic<bool> &stopFlag) {
  for (size_t nodeIndex = 0; nodeIndex < nodes.getNumberOfNodes();
       nodeIndex++) {
    dag::INode *currentNode = nodes.getNodeAt(nodeIndex);
    currentNode->execute();

    if (stopFlag) {
      break;
    }
  }
}

template <size_t NUM_OF_NODES>
void runNodeListSerialNTimes(dag::NodeList<NUM_OF_NODES> nodes,
                             std::atomic<bool> &stopFlag, size_t n) {
  for (int iter = 0; iter < n; iter++) {
    runNodeListSerialOnce(nodes, stopFlag);

    if (stopFlag) {
      break;
    }
  }
}

template <size_t NUM_OF_NODES>
void runNodeListSerialLoop(dag::NodeList<NUM_OF_NODES> nodes,
                           std::atomic<bool> &stopFlag) {
  while (!stopFlag) {
    runNodeListSerialOnce(nodes, stopFlag);
  }
}

// template <size_t GRAPH_SIZE, size_t NUMBER_OF_THREADS, size_t
// TASK_BUFFER_SIZE> void runGraphParallelOnce(dag::Dag<GRAPH_SIZE> &graph,
//                           std::atomic<bool> &stopFlag) {
//   const auto tasks = graph.getSortedTasks();

//   threadPool::ThreadPool<NUMBER_OF_THREADS, GRAPH_SIZE, TASK_BUFFER_SIZE>
//   pool{
//       tasks, 1};

//   pool.waitForAllTasks(stopFlag);
// }

// template <size_t GRAPH_SIZE, size_t NUMBER_OF_THREADS, size_t
// TASK_BUFFER_SIZE> void runGraphParallelNTimes(dag::Dag<GRAPH_SIZE> &graph,
//                             std::atomic<bool> &stopFlag, size_t n,
//                             bool waitForPrevious, const size_t timeoutMillis)
//                             {
//   const auto tasks = graph.getSortedTasks();

//   threadPool::ThreadPool<NUMBER_OF_THREADS, GRAPH_SIZE, TASK_BUFFER_SIZE>
//   pool{
//       tasks, n};

//   pool.waitForAllTasks(stopFlag);
// }

// template <size_t GRAPH_SIZE, size_t NUMBER_OF_THREADS, size_t
// TASK_BUFFER_SIZE> void runGraphParallelLoop(dag::Dag<GRAPH_SIZE> &graph,
//                           std::atomic<bool> &stopFlag, bool waitForPrevious,
//                           const size_t timeoutMillis) {
//   const auto tasks = graph.getSortedTasks();

//   threadPool::ThreadPool<NUMBER_OF_THREADS, GRAPH_SIZE, TASK_BUFFER_SIZE>
//   pool{
//       tasks};

//   pool.waitForAllTasks(stopFlag);
// }

} // namespace core

#endif // CORE_H
