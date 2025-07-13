#ifndef THREAD_TASK_H
#define THREAD_TASK_H
#include <string>

namespace threadPool {

using ThreadTaskFunction = void (*)(void *);

class IThreadTask {
public:
  virtual ~IThreadTask() = default;
  virtual void run() const = 0;
  virtual std::string name() const = 0;
};

class NullThreadTask final : public IThreadTask {
public:
  // NOLINTNEXTLINE
  NullThreadTask() {}
  // NOLINTNEXTLINE
  ~NullThreadTask() override {}

  void run() const override {}
  std::string name() const override { return "NullThreadTask"; }
};

const NullThreadTask nullThreadTask{};

} // namespace threadPool

#endif // THREAD_TASK_H
