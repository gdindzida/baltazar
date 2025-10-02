#ifndef BALTAZAR_THREAD_POOL_HPP
#define BALTAZAR_THREAD_POOL_HPP

#include "../utils/optional.hpp"
#include "thread_task.hpp"
#include "thread_task_queue.hpp"

#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <iostream>
#include <mutex>
#include <optional>
#include <thread>

namespace baltazar {
namespace threadPool {

template <size_t THREAD_NUM, size_t MAX_QUEUE_SIZE> class ThreadPool {
  std::array<std::thread, THREAD_NUM> m_threads;
  TaskQueue<MAX_QUEUE_SIZE> m_scheduledJobs;
  TaskQueue<MAX_QUEUE_SIZE> m_doneJobs;
  size_t m_numberOfTasks{0};
  size_t m_numberOfRunningTasks{0};
  std::mutex m_mtx;
  std::condition_variable m_addTaskCv;
  std::condition_variable m_finishTaskCv;
  std::condition_variable m_popedTaskCv;
  bool m_stop{false};

public:
  explicit ThreadPool() {
    for (int i = 0; i < THREAD_NUM; i++) {
      m_threads[i] = std::thread([this, i] {
        while (true) {
          std::unique_lock lock(m_mtx);
          m_addTaskCv.wait(
              lock, [this] { return !m_scheduledJobs.empty() || m_stop; });

          if (m_stop) {
            break;
          }

          auto job = m_scheduledJobs.pop().value();
          auto task = job._task;
          m_numberOfRunningTasks++;
          lock.unlock();

#ifdef DEBUGLOG
          std::cout << "[Thread" << i << "] "
                    << "Thread " << i << " has resources."
                    << "\n";

          std::cout << "[Thread" << i << "] "
                    << "Running a task = " << task->getIdentifier() << "\n";
#endif

#ifdef PROFILELOG
          job._startedTimePoint = std::chrono::steady_clock::now();
          job._threadId = static_cast<size_t>(i);
#endif

          task->run();

#ifdef PROFILELOG
          job._endedTimePoint = std::chrono::steady_clock::now();
#endif

          lock.lock();
          m_numberOfRunningTasks--;
          if (job._shouldSyncWhenDone) {
            bool success = m_doneJobs.push(job);
            assert(success && "Fatal error: mutex is locked twice.");
            lock.unlock();

            m_finishTaskCv.notify_all();

#ifdef DEBUGLOG
            std::cout << "[Thread" << i << "] "
                      << "Subbmiting task for sync = " << task->getIdentifier()
                      << "\n";
#endif
          } else {
            m_numberOfTasks--;
            lock.unlock();

            m_finishTaskCv.notify_all();
            m_popedTaskCv.notify_one();

#ifdef DEBUGLOG
            std::cout << "[Thread" << i << "] "
                      << "Discard task after finishing = "
                      << task->getIdentifier() << "\n";
#endif
          }
        }
      });
    }
  }

  ~ThreadPool() {
    this->shutdown();

    for (auto &th : m_threads) {
      if (th.joinable()) {
        th.join();
      }
    }
  }

  bool tryScheduleTask(ThreadJob job) {
    std::unique_lock lock(m_mtx);

    if (m_numberOfTasks >= MAX_QUEUE_SIZE) {
      return false;
    }

#ifdef PROFILELOG
    job._scheduledTimePoint = std::chrono::steady_clock::now();
#endif

    if (!m_scheduledJobs.push(job)) {
      return false;
    }

    m_numberOfTasks++;

#ifdef DEBUGLOG
    std::cout << "Scheduling task " << job._task->getIdentifier() << "\n";
#endif

    lock.unlock();
    m_addTaskCv.notify_one();

    return true;
  }

  bool scheduleTask(ThreadJob job) {
    std::unique_lock lock(m_mtx);

    m_popedTaskCv.wait(lock, [this] {
      return ((m_numberOfTasks < MAX_QUEUE_SIZE) && !m_scheduledJobs.full()) ||
             m_stop;
    });

    if (m_stop) {
      return false;
    }

#ifdef PROFILELOG
    job._scheduledTimePoint = std::chrono::steady_clock::now();
#endif

    m_numberOfTasks++;
    bool success = m_scheduledJobs.push(job);
    assert(success && "Fatal error: mutex is locked twice.");

#ifdef DEBUGLOG
    std::cout << "Scheduling task " << job._task->getIdentifier() << "\n";
#endif

    lock.unlock();
    m_addTaskCv.notify_one();

    return true;
  }

  utils::Optional<ThreadJob> tryGetNextDoneTask() {
    std::unique_lock lock(m_mtx);

    if (m_doneJobs.empty()) {
      return utils::Optional<ThreadJob>();
    }

    m_numberOfTasks--;
    ThreadJob job = m_doneJobs.pop().value();
    assert(job._task != nullptr &&
           "Fatal error: Null pointer pushed to done tasks.");

#ifdef DEBUGLOG
    std::cout << "Reporting done task " << job._task->getIdentifier() << "\n";
#endif

    lock.unlock();
    m_popedTaskCv.notify_one();

    return job;
  }

  utils::Optional<ThreadJob> getNextDoneTask() {
    std::unique_lock lock(m_mtx);

    m_finishTaskCv.wait(lock, [this] { return !m_doneJobs.empty() || m_stop; });

    if (m_stop) {
      return utils::Optional<ThreadJob>();
    }

    m_numberOfTasks--;
    ThreadJob job = m_doneJobs.pop().value();
    assert(job._task != nullptr &&
           "Fatal error: Null pointer pushed to done tasks.");

#ifdef DEBUGLOG
    std::cout << "Reporting done task " << job._task->getIdentifier() << "\n";
#endif

    lock.unlock();
    m_popedTaskCv.notify_one();

    return job;
  }

  void waitForAllTasks(std::atomic<bool> &stopFlag) {
    std::unique_lock lock(m_mtx);
    m_finishTaskCv.wait(lock, [this, &stopFlag] {
      m_stop = stopFlag;
      return (m_numberOfRunningTasks == 0 && m_scheduledJobs.empty()) || m_stop;
    });
  }

  void shutdown() {
    std::unique_lock lock(m_mtx);
    m_stop = true;

#ifdef DEBUGLOG
    std::cout << "Shutting down..."
              << "\n";
    std::cout << "Number of tasks = " << m_scheduledJobs.size() << "\n";
#endif

    lock.unlock();
    m_addTaskCv.notify_all();
  }
};
} // namespace threadPool
} // namespace baltazar

#endif // BALTAZAR_THREAD_POOL_HPP
