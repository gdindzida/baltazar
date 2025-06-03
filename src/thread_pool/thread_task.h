#ifndef THREAD_TASK_H
#define THREAD_TASK_H

#include <string>

namespace threadPool {

using taskFunction = void (*)(void *);

// NOLINTNEXTLINE
void nullTaskFunction(void *) {
  // do nothing
}

struct Task {
  taskFunction _func;
  std::string _name;
  taskFunction _initContext;
  taskFunction _deleteContext;
  void *_context;

  void initContext() const {
    if (_initContext != nullptr) {
      _initContext(_context);
    }
  }

  void deleteContext() const {
    if (_deleteContext != nullptr) {
      _deleteContext(_context);
    }
  }

  void run() const {
    if (_func != nullptr) {
      _func(_context);
    }
  }
};
} // namespace threadPool

#endif // THREAD_TASK_H
