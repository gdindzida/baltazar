#ifndef THREAD_TASK_H
#define THREAD_TASK_H

#include <chrono>
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
#ifdef PROFILELOG
  std::chrono::steady_clock::time_point _scheduledTimePoint;
  std::chrono::steady_clock::time_point _startedTimePoint;
  std::chrono::steady_clock::time_point _endedTimePoint;
  std::chrono::steady_clock::time_point _syncedTimePoint;
  size_t _threadId;
#endif
};

const NullThreadTask nullThreadTask{};

} // namespace threadPool

#endif // THREAD_TASK_H
