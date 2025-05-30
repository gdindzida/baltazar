#ifndef THREAD_TASK_H
#define THREAD_TASK_H

namespace thread_pool {

using TaskFn = void (*)(void *);

struct Task {
  TaskFn func = nullptr;
  void *data = nullptr;

  void run() {
    if (func)
      func(data);
  }
};
} // namespace thread_pool

#endif // THREAD_TASK_H
