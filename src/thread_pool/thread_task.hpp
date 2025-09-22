#ifndef THREAD_TASK_H
#define THREAD_TASK_H

#include <cstddef>
namespace threadPool {

using ThreadTaskFunction = void (*)(void *);

class IThreadTask {
public:
  virtual ~IThreadTask() = default;
  virtual void run() const = 0;
  virtual size_t getIdentifier() const = 0;
};

class NullThreadTask final : public IThreadTask {
public:
  // NOLINTNEXTLINE
  NullThreadTask() {}
  // NOLINTNEXTLINE
  ~NullThreadTask() override {}

  void run() const override {}
  size_t getIdentifier() const override { return 0UL; }
};

struct ThreadJob {
  IThreadTask *_task;
  size_t _id;
  bool _shouldSyncWhenDone;
};

const NullThreadTask nullThreadTask{};

} // namespace threadPool

#endif // THREAD_TASK_H
