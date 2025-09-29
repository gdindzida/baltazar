#ifndef FUNCTION_TRAITS_HPP
#define FUNCTION_TRAITS_HPP

#include <cstddef>
#include <tuple>

namespace baltazar {
namespace utils {

// Base case
template <typename T> struct FunctionTraits;

// Function
template <typename RET_TYPE, typename... ARGS>
struct FunctionTraits<RET_TYPE(ARGS...)> {
  using ReturnType = RET_TYPE;
  using ArgsTuple = std::tuple<ARGS...>;
  static constexpr size_t ArgsSize = sizeof...(ARGS);
};

// Function pointer
template <typename RET_TYPE, typename... ARGS>
struct FunctionTraits<RET_TYPE (*)(ARGS...)>
    : FunctionTraits<RET_TYPE(ARGS...)> {};

// Member pointer
template <typename CLASS, typename RET_TYPE, typename... ARGS>
struct FunctionTraits<RET_TYPE (CLASS::*)(ARGS...) const>
    : FunctionTraits<RET_TYPE(ARGS...)> {};

template <typename CLASS, typename RET_TYPE, typename... ARGS>
struct FunctionTraits<RET_TYPE (CLASS::*)(ARGS...)>
    : FunctionTraits<RET_TYPE(ARGS...)> {};

// Anything else
template <typename F>
struct FunctionTraits : FunctionTraits<decltype(&F::operator())> {};

} // namespace utils
} // namespace baltazar

#endif // FUNCTION_TRAITS_HPP
