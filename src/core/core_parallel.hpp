#ifndef BALTAZAR_CORE_PARALLEL_HPP
#define BALTAZAR_CORE_PARALLEL_HPP

#include "../core/profiling.hpp"
#include "../dag/dag.hpp"
#include "../thread_pool/thread_pool.hpp"
#include <array>
#include <atomic>
#include <ostream>

namespace baltazar {
namespace core {

template <typename ProfilerType = NullProfiler> class ParallelCoreRunner {
public:
  ParallelCoreRunner(std::ofstream *s = nullptr, bool profilerOn = false)
      : m_profiler(*s, profilerOn) {
    if constexpr (!std::is_same_v<std::decay_t<ProfilerType>, NullProfiler>) {
      assert(s != nullptr && "Profiler object provided is null.");
    }
  }

  template <size_t NUM_OF_NODES, size_t NUMBER_OF_THREADS,
            size_t TASK_BUFFER_SIZE>
  void runNodeListParallelOnce(
      dag::NodeList<NUM_OF_NODES> &nodes,
      threadPool::ThreadPool<NUMBER_OF_THREADS, TASK_BUFFER_SIZE> &tPool,
      std::atomic<bool> &stopFlag, ICoreProfiler *profiler = nullptr) {

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

      utils::Optional<threadPool::ThreadJob> doneJob =
          tPool.tryGetNextDoneTask();

      while (doneJob.has_value()) {
        dag::INode *doneNode = static_cast<dag::INode *>(doneJob.value()._task);
        doneNode->setDone();
        doneFlags[doneJob.value()._id] = true;
        numberOfTasksDone++;

#ifdef PROFILELOG
        doneJob.value()._syncedTimePoint = std::chrono::steady_clock::now();

        m_profiler.logJob(doneJob.value());
#endif

        doneJob = tPool.tryGetNextDoneTask();
      }
    }

    m_waveNumber++;
  }

  template <size_t NUM_OF_NODES, size_t NUMBER_OF_THREADS,
            size_t TASK_BUFFER_SIZE>
  void runNodeListParallelNTimes(
      dag::NodeList<NUM_OF_NODES> &nodes,
      threadPool::ThreadPool<NUMBER_OF_THREADS, TASK_BUFFER_SIZE> &tPool,
      std::atomic<bool> &stopFlag, size_t n,
      ICoreProfiler *profiler = nullptr) {
#ifdef PROFILELOG
    auto startRunTimePoint = std::chrono::steady_clock::now();
#endif
    for (int iter = 0; iter < n; iter++) {
#ifdef PROFILELOG
      auto startTimePoint = std::chrono::steady_clock::now();
#endif

      runNodeListParallelOnce(nodes, tPool, stopFlag);

#ifdef PROFILELOG
      auto endTimePoint = std::chrono::steady_clock::now();

      m_profiler.logWave(
          std::chrono::duration_cast<microsecs>(endTimePoint - startTimePoint),
          m_waveNumber);
#endif

      if (stopFlag) {
        break;
      }
    }
#ifdef PROFILELOG
    auto endRunTimePoint = std::chrono::steady_clock::now();
    m_profiler.logRun(std::chrono::duration_cast<microsecs>(endRunTimePoint -
                                                            startRunTimePoint));
#endif
  }

  template <size_t NUM_OF_NODES, size_t NUMBER_OF_THREADS,
            size_t TASK_BUFFER_SIZE>
  void runNodeListParallelLoop(
      dag::NodeList<NUM_OF_NODES> &nodes,
      threadPool::ThreadPool<NUMBER_OF_THREADS, TASK_BUFFER_SIZE> &tPool,
      std::atomic<bool> &stopFlag, ICoreProfiler *profiler = nullptr) {
#ifdef PROFILELOG
    auto startRunTimePoint = std::chrono::steady_clock::now();
#endif
    while (!stopFlag) {
#ifdef PROFILELOG
      auto startTimePoint = std::chrono::steady_clock::now();
#endif

      runNodeListParallelOnce(nodes, tPool, stopFlag);

#ifdef PROFILELOG
      auto endTimePoint = std::chrono::steady_clock::now();

      m_profiler.logWave(
          std::chrono::duration_cast<microsecs>(endTimePoint - startTimePoint),
          m_waveNumber);
#endif

      if (stopFlag) {
        break;
      }
    }
#ifdef PROFILELOG
    auto endRunTimePoint = std::chrono::steady_clock::now();
    m_profiler.logRun(std::chrono::duration_cast<microsecs>(endRunTimePoint -
                                                            startRunTimePoint));
#endif
  }

private:
  size_t m_waveNumber{0};
  ProfilerType m_profiler;
};

ParallelCoreRunner()->ParallelCoreRunner<NullProfiler>;

} // namespace core
} // namespace baltazar

#endif // BALTAZAR_CORE_PARALLEL_HPP
