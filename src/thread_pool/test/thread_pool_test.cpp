#include "../thread_pool.hpp"
#include <gtest/gtest.h>

class TestThreadTask final : public threadPool::IThreadTask {
public:
  explicit TestThreadTask(void *ctx) : m_context(ctx) {};

  // NOLINTNEXTLINE
  ~TestThreadTask() override {};

  void run() const override { helperTaskFunction(m_context); }

  std::string name() const override { return "TestThreadTask"; }

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
  threadPool::ThreadPool<numThreads, 0, 10> threadPool{};
  std::atomic stop{false};
  threadPool.waitForAllTasks(stop);

  // Assert
}

TEST(ThreadPoolTest, CreateThreadsWithOneTaskAndWaitForAll) {
  // Arrange
  constexpr size_t numThreads = 2;
  threadPool::ThreadPool<numThreads, 0, 10> threadPool{};
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

  threadPool::ThreadPool<numThreads, 0, 10> threadPool{};
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

  threadPool::ThreadPool<numThreads, 0, 10> threadPool{};
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

TEST(ThreadPoolTest, CreateThreadsWithPersitentTasksAndStop) {
  // Arrange
  constexpr size_t numThreads = 2;
  std::atomic<size_t> testCounter{0};

  // Act
  TestThreadTask task{&testCounter};
  threadPool::ThreadPool<numThreads, 3, 10> threadPool{{&task, &task, &task}};

  std::this_thread::sleep_for(std::chrono::milliseconds(4));

  threadPool.shutdown();

  // Assert
  EXPECT_LE(testCounter.load(), 4);
}

TEST(ThreadPoolTest, CreateThreadsWithPersitentTasksThenAddFewTasksAndStop) {
  // Arrange
  constexpr size_t numThreads = 2;
  std::atomic<size_t> testCounter{0};
  std::atomic<size_t> testCounter1{0};

  // Act
  TestThreadTask task{&testCounter};
  threadPool::ThreadPool<numThreads, 3, 10> threadPool{{&task, &task, &task}};

  TestThreadTask additionalTask{&testCounter1};
  threadPool.scheduleTask(&additionalTask);
  threadPool.scheduleTask(&additionalTask);

  std::this_thread::sleep_for(std::chrono::milliseconds(6));

  threadPool.shutdown();

  // Assert
  EXPECT_LE(testCounter.load(), 5);
  EXPECT_LE(testCounter1.load(), 2);
  EXPECT_GE(testCounter1.load(), 1);
}

TEST(ThreadPoolTest, CreateThreadsWithPersitentTasksNTimes) {
  // Arrange
  constexpr size_t numThreads = 2;
  constexpr size_t numOfLoops = 15;
  std::atomic<size_t> testCounter{0};

  // Act
  TestThreadTask task{&testCounter};
  threadPool::ThreadPool<numThreads, 3, 10> threadPool{{&task, &task, &task},
                                                       numOfLoops};

  std::atomic<bool> stop{false};
  threadPool.waitForAllTasks(stop);

  // Assert
  EXPECT_LE(testCounter.load(), 3 * numOfLoops);
}
