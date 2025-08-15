#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "thread_task_queue.hpp"

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

namespace threadPool {

template <size_t THREAD_NUM, size_t MAX_QUEUE_SIZE> class ThreadPool {
  std::array<std::thread, THREAD_NUM> m_threads;
  TaskQueue<MAX_QUEUE_SIZE> m_tasks;
  std::mutex m_mtx;
  std::condition_variable m_addTaskCv;
  std::condition_variable m_finishTaskCv;
  bool m_stop{false};

public:
  explicit ThreadPool() {
    for (int i = 0; i < THREAD_NUM; i++) {
      m_threads[i] = std::thread([this, i] {
        const IThreadTask *task = &nullThreadTask;
        while (true) {
          std::unique_lock lock(m_mtx);
          m_addTaskCv.wait(lock, [this] { return !m_tasks.empty() || m_stop; });

          if (m_stop) {
            return;
          }

#ifdef DEBUGLOG
          std::cout << "[Thread" << i << "] " << "Thread " << i
                    << " has resources." << "\n";

          // Currently all non-persistent tasks are prioritized.
          std::cout << "[Thread" << i << "] "
                    << "Running a task = " << task->name() << "\n";
#endif

          task = m_tasks.pop();

          lock.unlock();

          task->run();

          m_finishTaskCv.notify_all();
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

  bool scheduleTask(IThreadTask *task) {
    std::unique_lock lock(m_mtx);

    if (!m_tasks.push(task)) {
      return false;
    }

    lock.unlock();
    m_addTaskCv.notify_one();

    return true;
  }

  void waitForAllTasks(std::atomic<bool> &stopFlag) {
    std::unique_lock lock(m_mtx);
    m_finishTaskCv.wait(lock, [this, &stopFlag] {
      m_stop = stopFlag;
      return m_tasks.empty() || m_stop;
    });
  }

  void shutdown() {
    std::unique_lock lock(m_mtx);
    m_stop = true;

#ifdef DEBUGLOG
    std::cout << "Shutting down..." << "\n";
    std::cout << "No tasks = " << m_tasks.empty() << "\n";
#endif

    lock.unlock();
    m_addTaskCv.notify_all();
  }
};
} // namespace threadPool

#endif // THREAD_POOL_H
