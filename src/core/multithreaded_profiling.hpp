#ifndef BALTAZAR_MULTITHREADED_PROFILING_HPP
#define BALTAZAR_MULTITHREADED_PROFILING_HPP
#include "../thread_pool/thread_task.hpp"
#include "../thread_pool/thread_task_queue.hpp"
#include "profiling.hpp"
#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <thread>

namespace baltazar {
namespace core {

template <size_t QUEUE_SIZE>
class MultiThreadedCoreProfiler : public ICoreProfiler {
public:
  MultiThreadedCoreProfiler(std::ofstream &s, bool isOn)
      : m_out(s),
        m_loggingThread(&MultiThreadedCoreProfiler<QUEUE_SIZE>::processQueue,
                        this),
        m_on(isOn) {}

  MultiThreadedCoreProfiler(const MultiThreadedCoreProfiler &other) = delete;
  MultiThreadedCoreProfiler(MultiThreadedCoreProfiler &&other) = default;

  ~MultiThreadedCoreProfiler() {
    this->shutdown();

    if (m_loggingThread.joinable()) {
      m_loggingThread.join();
    }
  }

  void shutdown() {
    std::unique_lock lock(m_mtx);
    m_stop = true;

    lock.unlock();
    m_addTaskCv.notify_all();
  }

  void logJob(threadPool::ThreadJob &job) override {
    std::unique_lock lock(m_mtx);

    m_popedTaskCv.wait(lock, [this] { return !m_tasksToLog.full() || m_stop; });

    if (m_stop) {
      return;
    }

    bool success = m_tasksToLog.push(job);
    assert(success && "Fatal error: mutex is locked twice.");

    lock.unlock();
    m_addTaskCv.notify_one();
  }

  void logWave(microsecs waveTime, size_t waveNumber) override {
    std::lock_guard lg{m_mtx};
    logWaveFunction(m_out, waveNumber, waveTime);
  }

  void logRun(microsecs runTime) override { logRunFunction(m_out, runTime); }

  void logCustomDiff(microsecs totalTime, size_t customIdentifier) override {
    std::lock_guard lg{m_mtx};
    logCustomDurationFunction(m_out, customIdentifier, totalTime);
  }

  void turnOn() override { m_on = true; }

  void turnOff() override { m_on = false; }

private:
  void processQueue() {
    while (!m_stop) {
      std::unique_lock<std::mutex> lock(m_mtx);
      m_addTaskCv.wait(lock, [&] { return m_stop || !m_tasksToLog.empty(); });

      while (!m_tasksToLog.empty()) {
        utils::Optional<threadPool::ThreadJob> job = m_tasksToLog.pop();
        if (job.has_value()) {
          logJobFunction(m_out, job.value());
        }
      }
      m_out.flush();
      lock.unlock();
      m_popedTaskCv.notify_all();
    }
  }

  std::ofstream &m_out;
  std::thread m_loggingThread;
  threadPool::TaskQueue<QUEUE_SIZE> m_tasksToLog;
  std::mutex m_mtx;
  std::condition_variable m_addTaskCv;
  std::condition_variable m_popedTaskCv;
  bool m_stop{false};
  std::atomic<bool> m_on;
};

} // namespace core
} // namespace baltazar

#endif // BALTAZAR_MULTITHREADED_PROFILING_HPP
