#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "../utils/optional.hpp"
#include "thread_task.hpp"
#include "thread_task_queue.hpp"

#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <optional>
#include <thread>

namespace threadPool {

template <size_t THREAD_NUM, size_t MAX_QUEUE_SIZE> class ThreadPool {
  std::array<std::thread, THREAD_NUM> m_threads;
  TaskQueue<MAX_QUEUE_SIZE> m_scheduledTasks;
  TaskQueue<MAX_QUEUE_SIZE> m_doneTasks;
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
        NullThreadTask nullThreadTask{};
        const IThreadTask *task = &nullThreadTask;
        while (true) {
          std::unique_lock lock(m_mtx);
          m_addTaskCv.wait(
              lock, [this] { return !m_scheduledTasks.empty() || m_stop; });

          if (m_stop) {
            break;
          }

          task = m_scheduledTasks.pop();
          m_numberOfRunningTasks++;
          lock.unlock();

#ifdef DEBUGLOG
          std::cout << "[Thread" << i << "] "
                    << "Thread " << i << " has resources."
                    << "\n";

          std::cout << "[Thread" << i << "] "
                    << "Running a task = " << task->name() << "\n";
#endif

          task->run();

          lock.lock();
          m_numberOfRunningTasks--;
          if (task->shouldSyncWhenDone()) {
            bool success = m_doneTasks.push(task);
            assert(success);
            lock.unlock();

            m_finishTaskCv.notify_all();

#ifdef DEBUGLOG
            std::cout << "[Thread" << i << "] "
                      << "Subbmiting task for sync = " << task->name() << "\n";
#endif
          } else {
            m_numberOfTasks--;
            lock.unlock();

            m_finishTaskCv.notify_all();
            m_popedTaskCv.notify_one();

#ifdef DEBUGLOG
            std::cout << "[Thread" << i << "] "
                      << "Discard task after finishing = " << task->name()
                      << "\n";
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

  bool tryScheduleTask(IThreadTask *task) {
    std::unique_lock lock(m_mtx);

    if (m_numberOfTasks >= MAX_QUEUE_SIZE) {
      return false;
    }

    if (!m_scheduledTasks.push(task)) {
      return false;
    }

    m_numberOfTasks++;

#ifdef DEBUGLOG
    std::cout << "Scheduling task " << task->name() << "\n";
#endif

    lock.unlock();
    m_addTaskCv.notify_one();

    return true;
  }

  bool scheduleTask(IThreadTask *task) {
    std::unique_lock lock(m_mtx);

    m_popedTaskCv.wait(lock, [this] {
      return ((m_numberOfTasks < MAX_QUEUE_SIZE) && !m_scheduledTasks.full()) ||
             m_stop;
    });

    if (m_stop) {
      return false;
    }

    m_numberOfTasks++;
    bool success = m_scheduledTasks.push(task);
    assert(success);

#ifdef DEBUGLOG
    std::cout << "Scheduling task " << task->name() << "\n";
#endif

    lock.unlock();
    m_addTaskCv.notify_one();

    return true;
  }

  utils::Optional<const IThreadTask *> tryGetNextDoneTask() {
    std::unique_lock lock(m_mtx);

    if (m_doneTasks.empty()) {
      return utils::Optional<const IThreadTask *>();
    }

    m_numberOfTasks--;
    const IThreadTask *retTask = m_doneTasks.pop();
    assert(retTask != nullptr);

#ifdef DEBUGLOG
    std::cout << "Reporting done task " << retTask->name() << "\n";
#endif

    lock.unlock();
    m_popedTaskCv.notify_one();

    return retTask;
  }

  utils::Optional<const IThreadTask *> getNextDoneTask() {
    std::unique_lock lock(m_mtx);

    m_finishTaskCv.wait(lock,
                        [this] { return !m_doneTasks.empty() || m_stop; });

    if (m_stop) {
      return utils::Optional<const IThreadTask *>();
    }

    m_numberOfTasks--;
    const IThreadTask *retTask = m_doneTasks.pop();
    assert(retTask != nullptr);

#ifdef DEBUGLOG
    std::cout << "Reporting done task " << retTask->name() << "\n";
#endif

    lock.unlock();
    m_popedTaskCv.notify_one();

    return retTask;
  }

  void waitForAllTasks(std::atomic<bool> &stopFlag) {
    std::unique_lock lock(m_mtx);
    m_finishTaskCv.wait(lock, [this, &stopFlag] {
      m_stop = stopFlag;
      return (m_numberOfRunningTasks == 0 && m_scheduledTasks.empty()) ||
             m_stop;
    });
  }

  void shutdown() {
    std::unique_lock lock(m_mtx);
    m_stop = true;

#ifdef DEBUGLOG
    std::cout << "Shutting down..."
              << "\n";
    std::cout << "Number of tasks = " << m_scheduledTasks.size() << "\n";
#endif

    lock.unlock();
    m_addTaskCv.notify_all();
  }
};
} // namespace threadPool

#endif // THREAD_POOL_H
