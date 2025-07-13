#ifndef CORE_H
#define CORE_H
#include "../dag/dag.hpp"
#include "../thread_pool/thread_pool.hpp"
#include <atomic>

namespace core {
template <size_t GRAPH_SIZE>
void runGraphSerialOnce(dag::Dag<GRAPH_SIZE> &graph,
                        std::atomic<bool> &stopFlag) {
  const auto tasks = graph.getSortedTasks();

  for (auto *threadTaskPtr : tasks) {

    threadTaskPtr->run();

    if (stopFlag) {
      break;
    }
  }
}

template <size_t GRAPH_SIZE>
void runGraphSerialNTimes(dag::Dag<GRAPH_SIZE> &graph,
                          std::atomic<bool> &stopFlag, size_t n) {
  const auto tasks = graph.getSortedTasks();

  for (int iter = 0; iter < n; iter++) {
    for (auto *threadTaskPtr : tasks) {

      threadTaskPtr->run();

      if (stopFlag) {
        break;
      }
    }

    if (stopFlag) {
      break;
    }
  }
}

template <size_t GRAPH_SIZE>
void runGraphSerialLoop(dag::Dag<GRAPH_SIZE> &graph,
                        std::atomic<bool> &stopFlag) {
  const auto tasks = graph.getSortedTasks();

  while (!stopFlag) {
    for (auto *threadTaskPtr : tasks) {

      threadTaskPtr->run();
    }
  }
}

template <size_t GRAPH_SIZE, size_t NUMBER_OF_THREADS, size_t TASK_BUFFER_SIZE>
void runGraphParallelOnce(dag::Dag<GRAPH_SIZE> &graph,
                          std::atomic<bool> &stopFlag) {
  const auto tasks = graph.getSortedTasks();

  threadPool::ThreadPool<NUMBER_OF_THREADS, GRAPH_SIZE, TASK_BUFFER_SIZE> pool{
      tasks, 1};

  pool.waitForAllTasks(stopFlag);
}

template <size_t GRAPH_SIZE, size_t NUMBER_OF_THREADS, size_t TASK_BUFFER_SIZE>
void runGraphParallelNTimes(dag::Dag<GRAPH_SIZE> &graph,
                            std::atomic<bool> &stopFlag, size_t n,
                            bool waitForPrevious, const size_t timeoutMillis) {
  const auto tasks = graph.getSortedTasks();

  threadPool::ThreadPool<NUMBER_OF_THREADS, GRAPH_SIZE, TASK_BUFFER_SIZE> pool{
      tasks, n};

  pool.waitForAllTasks(stopFlag);
}

template <size_t GRAPH_SIZE, size_t NUMBER_OF_THREADS, size_t TASK_BUFFER_SIZE>
void runGraphParallelLoop(dag::Dag<GRAPH_SIZE> &graph,
                          std::atomic<bool> &stopFlag, bool waitForPrevious,
                          const size_t timeoutMillis) {
  const auto tasks = graph.getSortedTasks();

  threadPool::ThreadPool<NUMBER_OF_THREADS, GRAPH_SIZE, TASK_BUFFER_SIZE> pool{
      tasks};

  pool.waitForAllTasks(stopFlag);
}
} // namespace core

#endif // CORE_H
