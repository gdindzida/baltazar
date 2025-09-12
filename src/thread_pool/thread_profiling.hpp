
#ifndef THREAD_PROFILING_HPP
#define THREAD_PROFILING_HPP
#include "thread_task.hpp"
#include <chrono>
#include <string>

namespace threadPool {

class ThreadProfilingTaskWrapper final : public IThreadTask {
public:
  // NOLINTNEXTLINE
  ThreadProfilingTaskWrapper(IThreadTask *wrappedTask)
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
  std::string name() const override { return m_wrappedTask->name(); }

  bool shouldSyncWhenDone() const override {
    return m_wrappedTask->shouldSyncWhenDone();
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

private:
  IThreadTask *m_wrappedTask;
  std::chrono::steady_clock::time_point m_scheduledTimePoint;
  mutable std::chrono::steady_clock::time_point m_startedTimePoint;
  mutable std::chrono::steady_clock::time_point m_endedTimePoint;
};

} // namespace threadPool

#endif // THREAD_PROFILING_HPP
