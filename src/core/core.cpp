#include "core.h"

#include "../thread_pool/thread_pool.h"
#include <csignal>

namespace core {
template <size_t GRAPH_SIZE>
void runGraphSerialOnce(dag::Dag<GRAPH_SIZE> &graph,
                        std::atomic<bool> &stopFlag) {
  const auto *nodes = graph.getSortedTasks();

  for (auto *nodePtr : nodes) {
    const auto *threadTaskPtr =
        dynamic_cast<threadPool::IThreadTask *>(nodePtr);

    threadTaskPtr->run();

    if (stopFlag) {
      break;
    }
  }
}

template <size_t GRAPH_SIZE>
void runGraphSerialNTimes(dag::Dag<GRAPH_SIZE> &graph,
                          std::atomic<bool> &stopFlag, const size_t n) {
  const auto *nodes = graph.getSortedTasks();

  for (int iter = 0; iter < n; iter++) {
    for (auto *nodePtr : nodes) {
      const auto *threadTaskPtr =
          dynamic_cast<threadPool::IThreadTask *>(nodePtr);

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
  const auto *nodes = graph.getSortedTasks();

  while (!stopFlag) {
    for (auto *nodePtr : nodes) {
      const auto *threadTaskPtr =
          dynamic_cast<threadPool::IThreadTask *>(nodePtr);

      threadTaskPtr->run();
    }
  }
}

template <size_t GRAPH_SIZE, size_t NUMBER_OF_THREADS, size_t TASK_BUFFER_SIZE>
void runGraphParallelOnce(dag::Dag<GRAPH_SIZE> &graph,
                          std::atomic<bool> &stopFlag) {
  const auto *nodes = graph.getSortedTasks();

  threadPool::ThreadPool<NUMBER_OF_THREADS, TASK_BUFFER_SIZE> pool{};

  for (auto *nodePtr : nodes) {
    const auto *threadTaskPtr =
        dynamic_cast<threadPool::IThreadTask *>(nodePtr);

    pool.scheduleTask(threadTaskPtr);
  }

  pool.waitForAllTasks(stopFlag);
}

template <size_t GRAPH_SIZE, size_t NUMBER_OF_THREADS, size_t TASK_BUFFER_SIZE>
void runGraphParallelNTimes(dag::Dag<GRAPH_SIZE> &graph,
                            std::atomic<bool> &stopFlag, const size_t n,
                            bool waitForPrevious) {
  const auto *nodes = graph.getSortedTasks();

  threadPool::ThreadPool<NUMBER_OF_THREADS, TASK_BUFFER_SIZE> pool{};
  for (int iter = 0; iter < n; iter++) {
    for (auto *nodePtr : nodes) {
      const auto *threadTaskPtr =
          dynamic_cast<threadPool::IThreadTask *>(nodePtr);

      pool.scheduleTask(threadTaskPtr);
    }

    if (waitForPrevious) {
      pool.waitForAllTasks(stopFlag);
    } else if (stopFlag) {
      break;
    }
  }

  pool.waitForAllTasks(stopFlag);
}

template <size_t GRAPH_SIZE, size_t NUMBER_OF_THREADS, size_t TASK_BUFFER_SIZE>
void runGraphParallelLoop(dag::Dag<GRAPH_SIZE> &graph,
                          std::atomic<bool> &stopFlag, bool waitForPrevious) {
  const auto *nodes = graph.getSortedTasks();

  threadPool::ThreadPool<NUMBER_OF_THREADS, TASK_BUFFER_SIZE> pool{};
  while (!stopFlag) {
    for (auto *nodePtr : nodes) {
      const auto *threadTaskPtr =
          dynamic_cast<threadPool::IThreadTask *>(nodePtr);

      pool.scheduleTask(threadTaskPtr);
    }

    if (waitForPrevious) {
      pool.waitForAllTasks(stopFlag);
    }
  }

  pool.waitForAllTasks(stopFlag);
}

} // namespace core