#include "../thread_pool.hpp"
#include <chrono>
#include <gtest/gtest.h>

class TestThreadTask final : public threadPool::IThreadTask {
public:
  explicit TestThreadTask(void *ctx, bool shouldSyncWhenDone)
      : m_context(ctx), m_shouldSyncWhenDone(shouldSyncWhenDone){};

  // NOLINTNEXTLINE
  ~TestThreadTask() override{};

  void run() const override { helperTaskFunction(m_context); }

  std::string name() const override {
    if (!m_shouldSyncWhenDone) {
      return "TestThreadTask - NOT SYNCED";
    } else {
      return "TestThreadTask - SYNCED";
    }
  }

  bool shouldSyncWhenDone() const override { return m_shouldSyncWhenDone; }

private:
  static void helperTaskFunction(void *context) {
    if (context != nullptr) {
      auto *testCounter = static_cast<std::atomic<size_t> *>(context);
      ++*testCounter;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }

  void *m_context;
  bool m_shouldSyncWhenDone;
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
  TestThreadTask task{nullptr, false};

  // Act
  threadPool.scheduleTask(&task);
  std::atomic stop{false};
  threadPool.waitForAllTasks(stop);

  // Assert
}

TEST(ThreadPoolTest, CreateThreadsWithManyTasksAndWaitForAll) {
  // Arrange
  constexpr size_t numThreads = 2;
  constexpr size_t numOfTasks = 10;
  std::atomic<size_t> testCounter{0};

  threadPool::ThreadPool<numThreads, 10> threadPool{};
  TestThreadTask task{&testCounter, false};

  // Act
  int counter = 0;
  for (int i = 0; i < numOfTasks; i++) {
    if (threadPool.scheduleTask(&task)) {
      counter++;
    }
  }
  std::atomic stop{false};
  threadPool.waitForAllTasks(stop);

  // Assert
  EXPECT_EQ(counter, numOfTasks);
}

TEST(ThreadPoolTest, CreateThreadsWithManyTasksAndWaitForAllWithTimeout) {
  // Arrange
  constexpr size_t numThreads = 2;
  constexpr size_t numOfTasks = 10;
  std::atomic<size_t> testCounter{0};

  threadPool::ThreadPool<numThreads, 10> threadPool{};
  TestThreadTask task{&testCounter, false};

  // Act
  int counter = 0;
  for (int i = 0; i < numOfTasks; i++) {
    if (threadPool.scheduleTask(&task, std::chrono::milliseconds(2))) {
      counter++;
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
  constexpr size_t numOfTasks = 10;
  std::atomic<size_t> testCounter{0};

  threadPool::ThreadPool<numThreads, 10> threadPool{};
  TestThreadTask task{&testCounter, false};

  // Act
  int counter = 0;
  for (int i = 0; i < numOfTasks; i++) {
    if (threadPool.scheduleTask(&task)) {
      counter++;
    }
  }
  std::atomic stop{false};
  threadPool.waitForAllTasks(stop);
  stop.store(true);

  // Assert
  EXPECT_EQ(counter, numOfTasks);
}

TEST(ThreadPoolTest, TryCreateThreadsWithManyTasksAndWaitForAllButStopEarly) {
  // Arrange
  constexpr size_t numThreads = 2;
  constexpr size_t numOfTasks = 10;
  std::atomic<size_t> testCounter{0};

  threadPool::ThreadPool<numThreads, 10> threadPool{};
  TestThreadTask task{&testCounter, false};

  // Act
  int counter = 0;
  for (int i = 0; i < numOfTasks; i++) {
    if (threadPool.tryScheduleTask(&task)) {
      counter++;
    }
  }
  std::atomic stop{false};
  threadPool.waitForAllTasks(stop);
  stop.store(true);

  // Assert
  EXPECT_LE(counter, numOfTasks);
}

TEST(ThreadPoolTest, CreateThreadsWithManyTasksAndTryWaitForDoneTasks) {
  // Arrange
  constexpr size_t numThreads = 2;
  constexpr size_t numOfTasks = 10;
  std::atomic<size_t> testCounter{0};

  threadPool::ThreadPool<numThreads, 10> threadPool{};
  TestThreadTask task{&testCounter, false};
  TestThreadTask taskSynced{&testCounter, true};

  // Act
  int counter = 0;
  for (int i = 0; i < numOfTasks; i++) {
    TestThreadTask *scheduledTask = &task;
    if (i % 2 == 0) {
      scheduledTask = &taskSynced;
    }
    if (threadPool.scheduleTask(scheduledTask)) {
      counter++;
    }
  }
  std::atomic stop{false};
  threadPool.waitForAllTasks(stop);
  stop.store(true);

  int doneTaskCounter = 0;
  auto doneTask = threadPool.tryGetNextDoneTask();
  while (doneTask.has_value()) {
    doneTaskCounter++;
    doneTask = threadPool.tryGetNextDoneTask();
  }

  // Assert
  EXPECT_LE(counter, numOfTasks);
  EXPECT_EQ(doneTaskCounter, numOfTasks / 2);
}

TEST(ThreadPoolTest, CreateThreadsWithManyTasksAndWaitForDoneTasks) {
  // Arrange
  constexpr size_t numThreads = 2;
  constexpr size_t numOfTasks = 10;
  std::atomic<size_t> testCounter{0};

  threadPool::ThreadPool<numThreads, 10> threadPool{};
  TestThreadTask task{&testCounter, false};
  TestThreadTask taskSynced{&testCounter, true};

  // Act
  int counter = 0;
  for (int i = 0; i < numOfTasks; i++) {
    TestThreadTask *scheduledTask = &task;
    if (i % 2 == 0) {
      scheduledTask = &taskSynced;
    }
    if (threadPool.scheduleTask(scheduledTask)) {
      counter++;
    }
  }

  int doneTaskCounter = 0;
  for (int i = 0; i < numOfTasks / 2; i++) {
    doneTaskCounter++;
    auto doneTask = threadPool.getNextDoneTask();
    EXPECT_TRUE(doneTask.has_value());
  }

  std::atomic stop{false};
  threadPool.waitForAllTasks(stop);

  // Assert
  EXPECT_LE(counter, numOfTasks);
  EXPECT_EQ(doneTaskCounter, numOfTasks / 2);
}

TEST(ThreadPoolTest, CreateThreadsWithManyTasksAndWaitForDoneTasksWithTimeout) {
  // Arrange
  constexpr size_t numThreads = 2;
  constexpr size_t numOfTasks = 10;
  std::atomic<size_t> testCounter{0};

  threadPool::ThreadPool<numThreads, 10> threadPool{};
  TestThreadTask task{&testCounter, false};
  TestThreadTask taskSynced{&testCounter, true};

  // Act
  int counter = 0;
  for (int i = 0; i < numOfTasks; i++) {
    TestThreadTask *scheduledTask = &task;
    if (i % 2 == 0) {
      scheduledTask = &taskSynced;
    }
    if (threadPool.scheduleTask(scheduledTask)) {
      counter++;
    }
  }

  int doneTaskCounter = 0;
  for (int i = 0; i < numOfTasks / 2; i++) {
    doneTaskCounter++;
    auto doneTask = threadPool.getNextDoneTask(std::chrono::milliseconds(3));
    EXPECT_TRUE(doneTask.has_value());
  }

  std::atomic stop{false};
  threadPool.waitForAllTasks(stop);

  // Assert
  EXPECT_LE(counter, numOfTasks);
  EXPECT_EQ(doneTaskCounter, numOfTasks / 2);
}
