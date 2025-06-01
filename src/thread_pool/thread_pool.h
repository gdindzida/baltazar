#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "thread_task_queue.h"

#include <condition_variable>
#include <format>
#include <functional>
#include <mutex>
#include <thread>

namespace threadPool {
template <size_t THREAD_NUM, size_t MAX_QUEUE_SIZE> class ThreadPool {
private:
  std::array<std::jthread, THREAD_NUM> m_threads;
  TaskQueue<MAX_QUEUE_SIZE> m_tasks;
  std::mutex m_mtx;
  std::condition_variable m_addTaskCv;
  std::condition_variable m_finishTaskCv;
  bool m_stop;

public:
  explicit ThreadPool() : m_stop(false) {
    for (int i = 0; i < THREAD_NUM; i++) {
      m_threads[i] = std::jthread([this] {
        Task task{[] {}};
        while (true) {
          std::unique_lock lock(m_mtx);
          m_addTaskCv.wait(lock, [this] { return !m_tasks.empty() || m_stop; });
          if (m_stop)
            return;
          task = m_tasks.pop();
          lock.unlock();
          task.run();
          m_finishTaskCv.notify_all();
        }
      });
    }
  }

  ~ThreadPool() {
    std::unique_lock lock(m_mtx);
    m_stop = true;
    lock.unlock();
    m_addTaskCv.notify_all();

    for (auto &th : m_threads) {
      th.join();
    }
  }

  bool scheduleTask(Task &task) {
    std::unique_lock lock(m_mtx);

    if (!m_tasks.push(task)) {
      return false;
    }

    lock.unlock();
    m_addTaskCv.notify_one();

    return true;
  }

  void waitForAllTasks() {
    std::unique_lock lock(m_mtx);
    m_finishTaskCv.wait(lock, [this] { return m_tasks.empty() || m_stop; });
  }
};
} // namespace threadPool

#endif // THREAD_POOL_H
