#include "../thread_pool.hpp"
#include "thread_task.hpp"
#include <chrono>
#include <gtest/gtest.h>

class TestThreadTask final : public threadPool::IThreadTask {
public:
  explicit TestThreadTask(void *ctx, size_t identifier)
      : m_context(ctx), m_identifier(identifier){};

  // NOLINTNEXTLINE
  ~TestThreadTask() override{};

  void run() const override { helperTaskFunction(m_context); }

  size_t getIdentifier() const override { return m_identifier; }

private:
  static void helperTaskFunction(void *context) {
    if (context != nullptr) {
      auto *testCounter = static_cast<std::atomic<size_t> *>(context);
      ++*testCounter;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }

  void *m_context;
  size_t m_identifier;
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
  size_t identifier = 13;
  TestThreadTask task{nullptr, identifier};

  // Act
  threadPool.scheduleTask({&task, 0, false});
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
  size_t identifier = 13;
  TestThreadTask task{&testCounter, identifier};

  // Act
  int counter = 0;
  for (int i = 0; i < numOfTasks; i++) {
    if (threadPool.scheduleTask({&task, static_cast<size_t>(i), false})) {
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
  size_t identifier = 13;
  TestThreadTask task{&testCounter, identifier};

  // Act
  int counter = 0;
  for (int i = 0; i < numOfTasks; i++) {
    if (threadPool.scheduleTask({&task, static_cast<size_t>(i), false})) {
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
  size_t identifier = 13;
  TestThreadTask task{&testCounter, identifier};

  // Act
  int counter = 0;
  for (int i = 0; i < numOfTasks; i++) {
    if (threadPool.tryScheduleTask({&task, static_cast<size_t>(i), false})) {
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
  size_t identifier = 13;
  TestThreadTask task{&testCounter, identifier};
  size_t identifierSynced = 14;
  TestThreadTask taskSynced{&testCounter, identifierSynced};

  // Act
  int counter = 0;
  for (int i = 0; i < numOfTasks; i++) {
    TestThreadTask *scheduledTask = &task;
    threadPool::ThreadJob job = {scheduledTask, static_cast<size_t>(i), false};
    if (i % 2 == 0) {
      job._task = &taskSynced;
      job._shouldSyncWhenDone = true;
    }
    if (threadPool.scheduleTask(job)) {
      counter++;
    }
  }
  std::atomic stop{false};
  threadPool.waitForAllTasks(stop);
  stop.store(true);

  // Assert
  int doneTaskCounter = 0;
  auto doneTask = threadPool.tryGetNextDoneTask();
  while (doneTask.has_value()) {
    doneTaskCounter++;
    doneTask = threadPool.tryGetNextDoneTask();
    EXPECT_EQ(doneTask.value()._id % 2, 0);
  }
  EXPECT_LE(counter, numOfTasks);
  EXPECT_EQ(doneTaskCounter, numOfTasks / 2);
}

TEST(ThreadPoolTest, CreateThreadsWithManyTasksAndWaitForDoneTasks) {
  // Arrange
  constexpr size_t numThreads = 2;
  constexpr size_t numOfTasks = 10;
  std::atomic<size_t> testCounter{0};

  threadPool::ThreadPool<numThreads, 10> threadPool{};
  size_t identifier = 13;
  TestThreadTask task{&testCounter, identifier};
  size_t identifierSynced = 14;
  TestThreadTask taskSynced{&testCounter, identifierSynced};

  // Act
  int counter = 0;
  for (int i = 0; i < numOfTasks; i++) {
    TestThreadTask *scheduledTask = &task;
    threadPool::ThreadJob job = {scheduledTask, static_cast<size_t>(i), false};
    if (i % 2 == 0) {
      job._task = &taskSynced;
      job._shouldSyncWhenDone = true;
    }
    if (threadPool.scheduleTask(job)) {
      counter++;
    }
  }

  // Assert
  int doneTaskCounter = 0;
  for (int i = 0; i < numOfTasks / 2; i++) {
    doneTaskCounter++;
    auto doneTask = threadPool.getNextDoneTask();
    EXPECT_TRUE(doneTask.has_value());
    EXPECT_EQ(doneTask.value()._id % 2, 0);
  }

  std::atomic stop{false};
  threadPool.waitForAllTasks(stop);

  EXPECT_LE(counter, numOfTasks);
  EXPECT_EQ(doneTaskCounter, numOfTasks / 2);
}
