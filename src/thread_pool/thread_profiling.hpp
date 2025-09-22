
#ifndef THREAD_PROFILING_HPP
#define THREAD_PROFILING_HPP
#include "thread_task.hpp"
#include <chrono>

namespace threadPool {

class ThreadProfilingTaskWrapper final : public IThreadTask {
public:
  // NOLINTNEXTLINE
  ThreadProfilingTaskWrapper(IThreadTask *wrappedTask, size_t identifier)
      : m_wrappedTask(wrappedTask) {
    m_scheduledTimePoint = std::chrono::steady_clock::now();
  }
  // NOLINTNEXTLINE
  ~ThreadProfilingTaskWrapper() override {}

  void run() const override {
    m_startedTimePoint = std::chrono::steady_clock::now();
    m_wrappedTask->run();
    m_endedTimePoint = std::chrono::steady_clock::now();
  }

  std::chrono::steady_clock::time_point getScheduledTimePoint() {
    return m_scheduledTimePoint;
  }

  std::chrono::steady_clock::time_point getStartedTimePoint() {
    return m_startedTimePoint;
  }

  std::chrono::steady_clock::time_point getEndedTimePoint() {
    return m_endedTimePoint;
  }

  size_t getIdentifier() const override { return m_identifier; }

private:
  IThreadTask *m_wrappedTask;
  std::chrono::steady_clock::time_point m_scheduledTimePoint;
  mutable std::chrono::steady_clock::time_point m_startedTimePoint;
  mutable std::chrono::steady_clock::time_point m_endedTimePoint;
  size_t m_identifier;
};

} // namespace threadPool

#endif // THREAD_PROFILING_HPP
