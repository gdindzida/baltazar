#include "../thread_pool.h"
#include <gtest/gtest.h>

TEST(BaltazarTest, CreateThreadsWithNoTasksAndWaitForAll) {
  // Arrange
  constexpr size_t numThreads = 2;

  // Act
  thread_pool::ThreadPool<numThreads, 10> threadPool{};
  threadPool.waitForAllTasks();

  // Assert
}

TEST(BaltazarTest, CreateThreadsWithOneTaskAndWaitForAll) {
  // Arrange
  constexpr size_t numThreads = 2;
  thread_pool::ThreadPool<numThreads, 10> threadPool{};
  thread_pool::Task task{
      [](void *) { std::this_thread::sleep_for(std::chrono::milliseconds(2)); },
      nullptr};

  // Act
  threadPool.scheduleTask(task);
  threadPool.waitForAllTasks();

  // Assert
}

TEST(BaltazarTest, CreateThreadsWithManyTasksAndWaitForAll) {
  // Arrange
  constexpr size_t numThreads = 2;
  constexpr size_t numOfTries = 10;
  constexpr size_t numOfTasks = 300;
  thread_pool::ThreadPool<numThreads, 10> threadPool{};
  thread_pool::Task task{
      [](void *) { std::this_thread::sleep_for(std::chrono::milliseconds(2)); },
      nullptr};

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
