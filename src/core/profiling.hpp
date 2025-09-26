#ifndef PROFILING_HPP
#define PROFILING_HPP

#include "../thread_pool/thread_task.hpp"
#include <cstddef>
#include <fstream>
namespace core {

using microsecs = std::chrono::microseconds;

inline void logJobFunction(std::ofstream &ofs, threadPool::ThreadJob &job) {
#ifdef PROFILELOG
  auto scheduledTime = std::chrono::duration_cast<microsecs>(
      job._startedTimePoint - job._scheduledTimePoint);
  auto runningTime = std::chrono::duration_cast<microsecs>(
      job._endedTimePoint - job._startedTimePoint);
  auto waitingTime = std::chrono::duration_cast<microsecs>(
      job._syncedTimePoint - job._endedTimePoint);

  ofs << "J, " << job._task->getIdentifier() << ", " << job._id << ", "
      << job._threadId << ", " << scheduledTime.count() << ", "
      << runningTime.count() << ", " << waitingTime.count() << "\n";
#endif
}

inline void logWaveFunction(std::ofstream &ofs, size_t waveNumber,
                            microsecs duration) {
  ofs << "W, " << waveNumber << ", " << duration.count() << "\n";
}

inline void logCustomDurationFunction(std::ofstream &ofs, size_t identifer,
                                      microsecs duration) {
  ofs << "C, " << identifer << ", " << duration.count() << "\n";
}

class ICoreProfiler {
public:
  virtual ~ICoreProfiler() = default;

  virtual void logJob(threadPool::ThreadJob &job) = 0;
  virtual void logWave(microsecs waveTime, size_t waveNumber) = 0;
  virtual void logCustomDiff(microsecs totalTime, size_t customIdentifier) = 0;
  virtual void turnOn() = 0;
  virtual void turnOff() = 0;
};

template <size_t QUEUE_SIZE>
class SingleThreadedCoreProfiler : public ICoreProfiler {
public:
  SingleThreadedCoreProfiler(std::ofstream &s, bool isOn)
      : m_out(s), m_on(isOn) {}

  SingleThreadedCoreProfiler(const SingleThreadedCoreProfiler &other) = default;
  SingleThreadedCoreProfiler(SingleThreadedCoreProfiler &&other) = default;

  ~SingleThreadedCoreProfiler() {}

  void logJob(threadPool::ThreadJob &job) override {
    if (!m_on) {
      return;
    }

    logJobFunction(m_out, job);

    m_counter++;
    if (m_counter >= QUEUE_SIZE) {
      m_counter = 0;
      m_out.flush();
    }
  }

  void logWave(microsecs waveTime, size_t waveNumber) override {
    logWaveFunction(m_out, waveNumber, waveTime);
  }

  void logCustomDiff(microsecs totalTime, size_t customIdentifier) override {
    logCustomDurationFunction(m_out, customIdentifier, totalTime);
  }

  void turnOn() override { m_on = true; }

  void turnOff() override { m_on = false; }

private:
  std::ofstream &m_out;
  size_t m_counter{0};
  bool m_on;
};

class NullProfiler : public ICoreProfiler {
public:
  NullProfiler(std::ofstream &s, bool isOn) {}
  NullProfiler(const NullProfiler &other) = default;
  NullProfiler(NullProfiler &&other) = default;

  ~NullProfiler() {}

  void logJob(threadPool::ThreadJob &job) override {}

  void logWave(microsecs waveTime, size_t waveNumber) override {}

  void logCustomDiff(microsecs totalTime, size_t customIdentifier) override {}

  void turnOn() override {}

  void turnOff() override {}

private:
};

} // namespace core

#endif // PROFILING_HPP
