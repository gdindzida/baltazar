#ifndef THREAD_TASK_H
#define THREAD_TASK_H

#include <functional>
#include <string>

namespace threadPool {

struct Task {
  std::function<void()> _func;
  std::string _name;

  void run() const {
    if (_func)
      _func();
  }
};
} // namespace threadPool

#endif // THREAD_TASK_H
