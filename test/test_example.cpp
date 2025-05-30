#include "baltazar_core.hpp"
#include <gtest/gtest.h>

TEST(BaltazarTest, RunTest) {
  baltazar::runBaltazar();
  EXPECT_EQ(2 + 3, 5);
}