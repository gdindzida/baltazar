#include "../optional.hpp"

#include <gtest/gtest.h>
#include <optional>

template <typename T>
utils::Optional<T> helperFunc(const T &value, bool setValue) {
  if (setValue) {
    return value;
  }

  return utils::Optional<T>();
}

TEST(OptionalTest, HellperFuncReturnsGivenValue) {
  // Arrange
  int myValue = 2;

  // Act
  auto myRetrunValue = helperFunc(myValue, true);

  // Assert
  EXPECT_TRUE(myRetrunValue.has_value());
  EXPECT_EQ(myRetrunValue.value(), myValue);
}

TEST(OptionalTest, HellperFuncReturnsNullOpt) {
  // Arrange
  int myValue = 2;

  // Act
  auto myRetrunValue = helperFunc(myValue, false);

  // Assert
  EXPECT_FALSE(myRetrunValue.has_value());
}

TEST(OptionalTest, HellperFuncReturnsGivenValueAndThenResetsIt) {
  // Arrange
  int myValue = 2;

  // Act
  auto myRetrunValue = helperFunc(myValue, true);
  myRetrunValue.reset();

  // Assert
  EXPECT_FALSE(myRetrunValue.has_value());
}

TEST(OptionalTest, HellperFuncReturnsGivenValueAndThenSetsItToDIfferentValue) {
  // Arrange
  int myValue = 2;
  int otherValue = 3;

  // Act
  auto myRetrunValue = helperFunc(myValue, true);
  myRetrunValue = helperFunc(otherValue, true);

  // Assert
  EXPECT_TRUE(myRetrunValue.has_value());
  EXPECT_EQ(myRetrunValue.value(), otherValue);
}
