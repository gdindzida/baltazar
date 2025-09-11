#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include "thread_task.hpp"

#include <array>
#include <cstdio>

namespace threadPool {
template <size_t MAX_TASKS> class TaskQueue {
  std::array<const IThreadTask *, MAX_TASKS> m_tasks;
  size_t m_head = 0UL;
  size_t m_tail = 0UL;
  size_t m_size = 0UL;

public:
  [[nodiscard]] bool push(const IThreadTask *task) {
    if (m_size >= MAX_TASKS) {
      return false;
    }

    m_tasks[m_tail] = task;
    m_tail = (m_tail + 1UL) % MAX_TASKS;
    m_size++;
    return true;
  }

  const IThreadTask *pop() { // NOLINT
    const IThreadTask *out = &nullThreadTask;
    if (m_size <= 0UL) {
      return out;
    }

    out = m_tasks[m_head];
    m_head = (m_head + 1UL) % MAX_TASKS;
    m_size--;
    return out;
  }

  [[nodiscard]] bool empty() const { return m_size == 0UL; }

  [[nodiscard]] bool full() const { return m_size == MAX_TASKS; }

  size_t size() const { return m_size; }
};
} // namespace threadPool

#endif // TASK_QUEUE_H
