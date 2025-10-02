#ifndef BALTAZAR_CORE_SERIAL_HPP
#define BALTAZAR_CORE_SERIAL_HPP

#include "../core/profiling.hpp"
#include "../dag/dag.hpp"
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <ostream>
#include <type_traits>

namespace baltazar {
namespace core {

template <typename PROFILER_TYPE = NullProfiler> class SerialCoreRunner {
public:
  SerialCoreRunner(std::ofstream *s = nullptr, bool profilerOn = false)
      : m_profiler(*s, profilerOn) {
    if constexpr (!std::is_same_v<std::decay_t<PROFILER_TYPE>, NullProfiler>) {
      assert(s != nullptr && "Profiler object provided is null.");
    }
  }

  template <size_t NUM_OF_NODES>
  void runNodeListSerialOnce(dag::NodeList<NUM_OF_NODES> &nodes,
                             std::atomic<bool> &stopFlag) {
    for (size_t nodeIndex = 0; nodeIndex < nodes.getNumberOfNodes();
         nodeIndex++) {
      dag::INode *currentNode = nodes.getNodeAt(nodeIndex);

#ifdef PROFILELOG
      threadPool::ThreadJob job{};
      job._task = currentNode;
      job._id = nodeIndex;
      job._shouldSyncWhenDone = true;
      job._scheduledTimePoint = std::chrono::steady_clock::now();
      job._startedTimePoint = job._scheduledTimePoint;
      job._threadId = 0;
#endif

      currentNode->run();
      currentNode->setDone();

#ifdef PROFILELOG
      job._endedTimePoint = std::chrono::steady_clock::now();
      job._syncedTimePoint = job._endedTimePoint;
      m_profiler.logJob(job);
#endif

      if (stopFlag) {
        break;
      }
    }

    m_waveNumber++;
  }

  template <size_t NUM_OF_NODES>
  void runNodeListSerialNTimes(dag::NodeList<NUM_OF_NODES> &nodes,
                               std::atomic<bool> &stopFlag, size_t n) {
    for (int iter = 0; iter < n; iter++) {
#ifdef PROFILELOG
      auto startTimePoint = std::chrono::steady_clock::now();
#endif

      runNodeListSerialOnce(nodes, stopFlag);

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
  }

  template <size_t NUM_OF_NODES>
  void runNodeListSerialLoop(dag::NodeList<NUM_OF_NODES> &nodes,
                             std::atomic<bool> &stopFlag) {
    while (!stopFlag) {
#ifdef PROFILELOG
      auto startTimePoint = std::chrono::steady_clock::now();
#endif

      runNodeListSerialOnce(nodes, stopFlag);

#ifdef PROFILELOG
      auto endTimePoint = std::chrono::steady_clock::now();

      m_profiler.logWave(
          std::chrono::duration_cast<microsecs>(endTimePoint - startTimePoint),
          m_waveNumber);
#endif
    }
  }

private:
  size_t m_waveNumber{0};
  PROFILER_TYPE m_profiler;
};

SerialCoreRunner()->SerialCoreRunner<NullProfiler>;

} // namespace core
} // namespace baltazar

#endif // BALTAZAR_CORE_SERIAL_HPP
