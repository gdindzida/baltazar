#include "../thread_pool.h"
#include <gtest/gtest.h>

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
  threadPool::Task task{
      [] { std::this_thread::sleep_for(std::chrono::milliseconds(2)); }};

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
  threadPool::Task task{[&testCounter] {
    ++testCounter;
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }};

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
  EXPECT_EQ(numOfTasks, counter);
}
