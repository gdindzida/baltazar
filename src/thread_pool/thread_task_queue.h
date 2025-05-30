#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include "thread_task.h"

#include <array>
#include <cstdio>

namespace thread_pool {
template <size_t MAX_TASKS> class TaskQueue {
  std::array<Task, MAX_TASKS> m_tasks;
  size_t m_head = 0;
  size_t m_tail = 0;
  size_t m_size = 0;

public:
  bool push(const Task &task) {
    if (m_size >= MAX_TASKS) {
      return false;
    }

    m_tasks[m_tail] = task;
    m_tail = (m_tail + 1) % MAX_TASKS;
    m_size++;
    return true;
  }

  Task pop() {
    Task out = {nullptr, nullptr};
    if (m_size <= 0) {
      return out;
    }

    out = m_tasks[m_head];
    m_head = (m_head + 1) % MAX_TASKS;
    m_size--;
    return out;
  }

  bool empty() const { return m_size == 0; }
};
} // namespace thread_pool

#endif // TASK_QUEUE_H
