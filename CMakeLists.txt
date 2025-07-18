cmake_minimum_required(VERSION 3.14)
project(baltazar)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(CTest)
include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
)
FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.8.4
)
set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "Don't treat warnings as errors")
FetchContent_MakeAvailable(googletest benchmark)

# add_subdirectory(benchmark/)
# add_subdirectory(examples/)
# add_subdirectory(test/)
add_subdirectory(src/)

