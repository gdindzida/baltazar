#include "../function_traits.hpp"

#include <gtest/gtest.h>
#include <string>
#include <type_traits>
#include <vector>

namespace baltazar {

TEST(FunctionalTraitsTest, TestExtractionOfTraitsOfSomeFunction) {
  // Arrange

  // Act
  using traits = utils::FunctionTraits<int(int, double)>;
  using ret = traits::ReturnType;
  using args = traits::ArgsTuple;
  constexpr size_t argsSize = traits::ArgsSize;

  using arg0 = std::tuple_element_t<0, args>;
  using arg1 = std::tuple_element_t<1, args>;

  // Assert
  constexpr bool retMatches = std::is_convertible_v<ret, int>;
  constexpr bool arg0Matches = std::is_convertible_v<arg0, int>;
  constexpr bool arg1Matches = std::is_convertible_v<arg1, double>;
  EXPECT_TRUE(retMatches);
  EXPECT_TRUE(arg0Matches);
  EXPECT_TRUE(arg1Matches);
  EXPECT_EQ(argsSize, 2);
}

TEST(FunctionalTraitsTest, TestExtractionOfTraitsOfSomeFunctionPointer) {
  // Arrange

  // Act
  using traits = utils::FunctionTraits<std::string (*)(float, int, double)>;
  using ret = traits::ReturnType;
  using args = traits::ArgsTuple;
  constexpr size_t argsSize = traits::ArgsSize;

  using arg0 = std::tuple_element_t<0, args>;
  using arg1 = std::tuple_element_t<1, args>;
  using arg2 = std::tuple_element_t<2, args>;

  // Assert
  constexpr bool retMatches = std::is_convertible_v<ret, std::string>;
  constexpr bool arg0Matches = std::is_convertible_v<arg0, float>;
  constexpr bool arg1Matches = std::is_convertible_v<arg1, int>;
  constexpr bool arg2Matches = std::is_convertible_v<arg2, double>;
  EXPECT_TRUE(retMatches);
  EXPECT_TRUE(arg0Matches);
  EXPECT_TRUE(arg1Matches);
  EXPECT_TRUE(arg2Matches);
  EXPECT_EQ(argsSize, 3);
}

TEST(FunctionalTraitsTest, TestExtractionOfTraitsOfSomeLambdaFunction) {
  // Arrange
  int context = 2;
  auto myLambdaFunc = [&context](short, std::vector<int>) { return 1; };

  // Act
  using traits = utils::FunctionTraits<decltype(myLambdaFunc)>;
  using ret = traits::ReturnType;
  using args = traits::ArgsTuple;
  constexpr size_t argsSize = traits::ArgsSize;

  using arg0 = std::tuple_element_t<0, args>;
  using arg1 = std::tuple_element_t<1, args>;

  // Assert
  constexpr bool retMatches = std::is_convertible_v<ret, int>;
  constexpr bool arg0Matches = std::is_convertible_v<arg0, short>;
  constexpr bool arg1Matches = std::is_convertible_v<arg1, std::vector<int>>;
  EXPECT_TRUE(retMatches);
  EXPECT_TRUE(arg0Matches);
  EXPECT_TRUE(arg1Matches);
  EXPECT_EQ(argsSize, 2);
}

TEST(FunctionalTraitsTest, TestExtractionOfTraitsOfSomeFunctor) {
  // Arrange
  struct MyFunctor {
    std::vector<int> operator()(double a, char b) const {
      return {static_cast<int>(a), b};
    }
  };

  // Act
  using traits = utils::FunctionTraits<MyFunctor>;
  using ret = traits::ReturnType;
  using args = traits::ArgsTuple;
  constexpr size_t argsSize = traits::ArgsSize;

  using arg0 = std::tuple_element_t<0, args>;
  using arg1 = std::tuple_element_t<1, args>;

  // Assert
  constexpr bool retMatches = std::is_convertible_v<ret, std::vector<int>>;
  constexpr bool arg0Matches = std::is_convertible_v<arg0, double>;
  constexpr bool arg1Matches = std::is_convertible_v<arg1, char>;
  EXPECT_TRUE(retMatches);
  EXPECT_TRUE(arg0Matches);
  EXPECT_TRUE(arg1Matches);
  EXPECT_EQ(argsSize, 2);
}

} // namespace baltazar
