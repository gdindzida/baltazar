#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include "../utils/optional.hpp"
#include "thread_task.hpp"

#include <array>
#include <cstdio>

namespace baltazar {
namespace threadPool {
template <size_t MAX_TASKS> class TaskQueue {
  std::array<ThreadJob, MAX_TASKS> m_tasks{};
  size_t m_head = 0UL;
  size_t m_tail = 0UL;
  size_t m_size = 0UL;

public:
  TaskQueue() = default;

  [[nodiscard]] bool push(const ThreadJob task) {
    if (m_size >= MAX_TASKS) {
      return false;
    }

    m_tasks[m_tail] = task;
    m_tail = (m_tail + 1UL) % MAX_TASKS;
    m_size++;
    return true;
  }

  const utils::Optional<ThreadJob> pop() { // NOLINT
    if (m_size <= 0UL) {
      return utils::Optional<ThreadJob>();
    }

    ThreadJob out = m_tasks[m_head];
    m_head = (m_head + 1UL) % MAX_TASKS;
    m_size--;
    return out;
  }

  [[nodiscard]] bool empty() const { return m_size == 0UL; }

  [[nodiscard]] bool full() const { return m_size == MAX_TASKS; }

  size_t size() const { return m_size; }
};
} // namespace threadPool
} // namespace baltazar

#endif // TASK_QUEUE_H
