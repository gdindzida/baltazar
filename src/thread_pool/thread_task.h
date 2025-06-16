#ifndef THREAD_TASK_H
#define THREAD_TASK_H

#include <string>

namespace threadPool {

using ThreadTaskFunction = void (*)(void *);

class IThreadTask {
public:
  virtual ~IThreadTask() = default;
  virtual void run() const = 0;
};

class NullThreadTask final : public IThreadTask {
public:
  // NOLINTNEXTLINE
  NullThreadTask() {}
  // NOLINTNEXTLINE
  ~NullThreadTask() override {}

  void run() const override {}
};

const NullThreadTask nullTreadTask{};

} // namespace threadPool

#endif // THREAD_TASK_H
