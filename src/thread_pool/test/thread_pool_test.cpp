#include "../thread_pool.h"
#include <gtest/gtest.h>

void helperTaskFunction(void *context) {
  if (context != nullptr) {
    auto *testCounter = static_cast<std::atomic<size_t> *>(context);
    ++*testCounter;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(2));
}

TEST(ThreadPoolTest, CreateThreadsWithNoTasksAndWaitForAll) {
  // Arrange
  constexpr size_t numThreads = 2;

  // Act
  threadPool::ThreadPool<numThreads, 10> threadPool{};
  threadPool.waitForAllTasks();

  // Assert
}

TEST(ThreadPoolTest, CreateThreadsWithOneTaskAndWaitForAll) {
  // Arrange
  constexpr size_t numThreads = 2;
  threadPool::ThreadPool<numThreads, 10> threadPool{};
  threadPool::Task task{helperTaskFunction};
  task._context = nullptr;

  // Act
  threadPool.scheduleTask(task);
  threadPool.waitForAllTasks();

  // Assert
}

TEST(ThreadPoolTest, CreateThreadsWithManyTasksAndWaitForAll) {
  // Arrange
  constexpr size_t numThreads = 2;
  constexpr size_t numOfTries = 10;
  constexpr size_t numOfTasks = 300;
  std::atomic<size_t> testCounter{0};

  threadPool::ThreadPool<numThreads, 10> threadPool{};
  threadPool::Task task{helperTaskFunction};
  task._context = static_cast<void *>(&testCounter);

  // Act
  int counter = 0;
  for (int i = 0; i < numOfTasks; i++) {
    for (int j = 0; j < numOfTries; j++) {
      if (threadPool.scheduleTask(task)) {
        counter++;
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
  }
  threadPool.waitForAllTasks();

  // Assert
  EXPECT_EQ(counter, numOfTasks);
}
