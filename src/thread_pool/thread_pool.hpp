#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "thread_task_queue.hpp"

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

namespace threadPool {

template <size_t THREAD_NUM, size_t PERSISTENT_QUEUE_SIZE,
          size_t MAX_QUEUE_SIZE>
class ThreadPool {
  std::array<std::thread, THREAD_NUM> m_threads;
  std::array<IThreadTask *, PERSISTENT_QUEUE_SIZE> m_persistentTasks;
  std::atomic<size_t> m_numberOfPersistentTasksDoneInCurrentIteration{
      PERSISTENT_QUEUE_SIZE};
  size_t m_currentPersistentTaskIndex{0};
  int64_t m_currentPersistentLoopIteration{0};
  int64_t m_maxNumberOfPersistentLoopIterations{-1};
  TaskQueue<MAX_QUEUE_SIZE> m_tasks;
  std::mutex m_mtx;
  std::mutex m_mtxLoop;
  std::condition_variable m_addTaskCv;
  std::condition_variable m_finishTaskCv;
  std::condition_variable m_finishedLoopCv;
  bool m_stop{false};
  bool firstLoop{true};

  void init() {
    for (int i = 0; i < THREAD_NUM; i++) {
      m_threads[i] = std::thread([this, i] {
        const IThreadTask *task = &nullThreadTask;
        while (true) {
          std::unique_lock lock(m_mtx);
          m_addTaskCv.wait(lock, [this] {
            return (PERSISTENT_QUEUE_SIZE > 0) || !m_tasks.empty() || m_stop;
          });
          if (m_stop) {
            return;
          }

          std::cout << "[Thread" << i << "] " << "Thread " << i
                    << " has resources." << "\n";

          bool currentTaskIsPersistent = false;

          // Currently all non-persistent tasks are prioritized.
          if (!m_tasks.empty()) {
            std::cout << "[Thread" << i << "] "
                      << "Running non persistent task = " << task->name()
                      << "\n";
            task = m_tasks.pop();
          } else {
            std::cout << "[Thread" << i << "] "
                      << "Trying to run persistent task. id="
                      << m_currentPersistentTaskIndex << "\n";

            if (m_currentPersistentTaskIndex == 0) {
              std::cout << "[Thread" << i << "] "
                        << "Trying to run new iteration. "
                        << m_currentPersistentLoopIteration << " / "
                        << m_maxNumberOfPersistentLoopIterations << "\n";

              if (!firstLoop &&
                  m_numberOfPersistentTasksDoneInCurrentIteration <
                      PERSISTENT_QUEUE_SIZE) {
                std::cout << "[Thread" << i << "] "
                          << "Waiting for previous iteration to finish..."
                          << "\n";

                m_finishedLoopCv.wait(lock, [this] { return true; });

                lock.unlock();

                std::cout << "[Thread" << i << "] "
                          << "Finished waiting!!!"
                          << "\n";

                continue;
              }

              if (firstLoop) {
                firstLoop = false;
              }

              if (m_maxNumberOfPersistentLoopIterations > 0) {
                if (m_currentPersistentLoopIteration <
                    m_maxNumberOfPersistentLoopIterations) {
                  m_currentPersistentLoopIteration++;
                } else {
                  std::cout << "[Thread" << i << "] "
                            << "All persistent loops are done."
                            << "\n";
                  lock.unlock();
                  continue;
                }
              }

              std::cout << "[Thread" << i << "] "
                        << "Starting new iteration." << "\n";

              // reset number of tasks done.
              m_numberOfPersistentTasksDoneInCurrentIteration = 0;
            }

            task = m_persistentTasks[m_currentPersistentTaskIndex];
            currentTaskIsPersistent = true;

            std::cout << "[Thread" << i << "] "
                      << "Running persistent task =" << task->name() << "\n";

            m_currentPersistentTaskIndex++;
            m_currentPersistentTaskIndex %= PERSISTENT_QUEUE_SIZE;
          }

          lock.unlock();
          task->run();
          m_finishTaskCv.notify_all();
          if (currentTaskIsPersistent) {
            ++m_numberOfPersistentTasksDoneInCurrentIteration;

            if (m_numberOfPersistentTasksDoneInCurrentIteration >=
                PERSISTENT_QUEUE_SIZE) {
              m_finishedLoopCv.notify_all();
            }
          }

          std::cout << "[Thread" << i << "] " << task->name() << " done. "
                    << m_numberOfPersistentTasksDoneInCurrentIteration << " / "
                    << PERSISTENT_QUEUE_SIZE << "\n";
        }
      });
    }
  }

  bool isPersistentLoopDone() const {
    return (m_currentPersistentLoopIteration >=
            m_maxNumberOfPersistentLoopIterations) &&
           (m_numberOfPersistentTasksDoneInCurrentIteration >=
            PERSISTENT_QUEUE_SIZE);
  }

public:
  explicit ThreadPool(
      const std::array<IThreadTask *, PERSISTENT_QUEUE_SIZE> &persistentTasks,
      int64_t maxNumberOfPersistentLoops = -1) {
    static_assert(PERSISTENT_QUEUE_SIZE > 0);

    for (size_t i = 0; i < PERSISTENT_QUEUE_SIZE; ++i) {
      m_persistentTasks[i] = persistentTasks[i];
    }

    m_maxNumberOfPersistentLoopIterations = maxNumberOfPersistentLoops;

    init();
  }

  explicit ThreadPool() {
    static_assert(PERSISTENT_QUEUE_SIZE == 0);
    init();
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
      return (m_tasks.empty() && isPersistentLoopDone()) || m_stop;
    });
  }

  void shutdown() {
    std::unique_lock lock(m_mtx);
    m_stop = true;

    std::cout << "Shutting down..." << "\n";
    std::cout << "No persistent tasks = " << m_tasks.empty() << "\n";
    std::cout << "Is persistent loop done = " << isPersistentLoopDone() << "\n";

    lock.unlock();
    m_addTaskCv.notify_all();
  }
};
} // namespace threadPool

#endif // THREAD_POOL_H
