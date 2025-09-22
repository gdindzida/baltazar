#ifndef CORE_H
#define CORE_H
#include "../dag/dag.hpp"
#include "../thread_pool/thread_pool.hpp"
#include <array>
#include <atomic>
#include <ostream>

namespace core {

template <size_t NUM_OF_NODES>
void runNodeListSerialOnce(dag::NodeList<NUM_OF_NODES> &nodes,
                           std::atomic<bool> &stopFlag) {
  for (size_t nodeIndex = 0; nodeIndex < nodes.getNumberOfNodes();
       nodeIndex++) {
    dag::INode *currentNode = nodes.getNodeAt(nodeIndex);
    currentNode->run();
    currentNode->setDone();

    if (stopFlag) {
      break;
    }
  }
}

template <size_t NUM_OF_NODES>
void runNodeListSerialNTimes(dag::NodeList<NUM_OF_NODES> &nodes,
                             std::atomic<bool> &stopFlag, size_t n) {
  for (int iter = 0; iter < n; iter++) {
    runNodeListSerialOnce(nodes, stopFlag);

    if (stopFlag) {
      break;
    }
  }
}

template <size_t NUM_OF_NODES>
void runNodeListSerialLoop(dag::NodeList<NUM_OF_NODES> &nodes,
                           std::atomic<bool> &stopFlag) {
  while (!stopFlag) {
    runNodeListSerialOnce(nodes, stopFlag);
  }
}

template <size_t NUM_OF_NODES, size_t NUMBER_OF_THREADS,
          size_t TASK_BUFFER_SIZE>
void runNodeListParallelOnce(
    dag::NodeList<NUM_OF_NODES> &nodes,
    threadPool::ThreadPool<NUMBER_OF_THREADS, TASK_BUFFER_SIZE> &tPool,
    std::atomic<bool> &stopFlag) {

  std::array<bool, NUM_OF_NODES> doneFlags{};
  std::array<bool, NUM_OF_NODES> scheduledFlags{};

  for (size_t nodeIndex = 0; nodeIndex < nodes.getNumberOfNodes();
       nodeIndex++) {
    nodes.getNodeAt(nodeIndex)->reset();
  }

  size_t numberOfTasksDone = 0;
  while (!stopFlag && (numberOfTasksDone < nodes.getNumberOfNodes())) {
    for (size_t nodeIndex = 0; nodeIndex < nodes.getNumberOfNodes();
         nodeIndex++) {
      dag::INode *node = nodes.getNodeAt(nodeIndex);
      if (node->isReady() && !doneFlags[nodeIndex] &&
          !scheduledFlags[nodeIndex]) {
        tPool.scheduleTask({node, nodeIndex, true});
        scheduledFlags[nodeIndex] = true;
      }

      if (stopFlag) {
        break;
      }
    }

    if (stopFlag) {
      break;
    }

    utils::Optional<const threadPool::ThreadJob> doneTask =
        tPool.tryGetNextDoneTask();

    while (doneTask.has_value()) {
      dag::INode *doneNode = static_cast<dag::INode *>(doneTask.value()._task);
      doneNode->setDone();
      doneFlags[doneTask.value()._id] = true;
      numberOfTasksDone++;

      doneTask = tPool.tryGetNextDoneTask();
    }
  }
}

template <size_t NUM_OF_NODES, size_t NUMBER_OF_THREADS,
          size_t TASK_BUFFER_SIZE>
void runNodeListParallelNTimes(
    dag::NodeList<NUM_OF_NODES> &nodes,
    threadPool::ThreadPool<NUMBER_OF_THREADS, TASK_BUFFER_SIZE> &tPool,
    std::atomic<bool> &stopFlag, size_t n) {
  for (int iter = 0; iter < n; iter++) {
    runNodeListParallelOnce(nodes, tPool, stopFlag);

    if (stopFlag) {
      break;
    }
  }
}
template <size_t NUM_OF_NODES, size_t NUMBER_OF_THREADS,
          size_t TASK_BUFFER_SIZE>
void runNodeListParallelLoop(
    dag::NodeList<NUM_OF_NODES> &nodes,
    threadPool::ThreadPool<NUMBER_OF_THREADS, TASK_BUFFER_SIZE> &tPool,
    std::atomic<bool> &stopFlag) {
  while (!stopFlag) {
    runNodeListParallelOnce(nodes, tPool, stopFlag);

    if (stopFlag) {
      break;
    }
  }
}

} // namespace core

#endif // CORE_H
