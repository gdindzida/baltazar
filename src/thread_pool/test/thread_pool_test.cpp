#include "../thread_pool.h"
#include <gtest/gtest.h>

class TestThreadTask final : public threadPool::IThreadTask {
public:
  explicit TestThreadTask(void *ctx) : m_context(ctx) {};

  // NOLINTNEXTLINE
  ~TestThreadTask() override {};

  void run() const override { helperTaskFunction(m_context); }

private:
  static void helperTaskFunction(void *context) {
    if (context != nullptr) {
      auto *testCounter = static_cast<std::atomic<size_t> *>(context);
      ++*testCounter;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }

  void *m_context;
};

TEST(ThreadPoolTest, CreateThreadsWithNoTasksAndWaitForAll) {
  // Arrange
  constexpr size_t numThreads = 2;

  // Act
  threadPool::ThreadPool<numThreads, 10> threadPool{};
  std::atomic stop{false};
  threadPool.waitForAllTasks(stop);

  // Assert
}

TEST(ThreadPoolTest, CreateThreadsWithOneTaskAndWaitForAll) {
  // Arrange
  constexpr size_t numThreads = 2;
  threadPool::ThreadPool<numThreads, 10> threadPool{};
  TestThreadTask task{nullptr};

  // Act
  threadPool.scheduleTask(&task);
  std::atomic stop{false};
  threadPool.waitForAllTasks(stop);

  // Assert
}

TEST(ThreadPoolTest, CreateThreadsWithManyTasksAndWaitForAll) {
  // Arrange
  constexpr size_t numThreads = 2;
  constexpr size_t numOfTries = 10;
  constexpr size_t numOfTasks = 300;
  std::atomic<size_t> testCounter{0};

  threadPool::ThreadPool<numThreads, 10> threadPool{};
  TestThreadTask task{&testCounter};

  // Act
  int counter = 0;
  for (int i = 0; i < numOfTasks; i++) {
    for (int j = 0; j < numOfTries; j++) {
      if (threadPool.scheduleTask(&task)) {
        counter++;
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
  }
  std::atomic stop{false};
  threadPool.waitForAllTasks(stop);

  // Assert
  EXPECT_EQ(counter, numOfTasks);
}

TEST(ThreadPoolTest, CreateThreadsWithManyTasksAndWaitForAllButStopEarly) {
  // Arrange
  constexpr size_t numThreads = 2;
  constexpr size_t numOfTries = 10;
  constexpr size_t numOfTasks = 300;
  std::atomic<size_t> testCounter{0};

  threadPool::ThreadPool<numThreads, 10> threadPool{};
  TestThreadTask task{&testCounter};

  // Act
  int counter = 0;
  for (int i = 0; i < numOfTasks; i++) {
    for (int j = 0; j < numOfTries; j++) {
      if (threadPool.scheduleTask(&task)) {
        counter++;
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
  }
  std::atomic stop{false};
  threadPool.waitForAllTasks(stop);
  stop.store(true);

  // Assert
  EXPECT_LE(counter, numOfTasks);
}
