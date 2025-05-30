add_library(baltazar_core core.cpp
        ../thread_pool/thread_pool.h
        ../thread_pool/benchmark/benchmark.cpp)

target_include_directories(baltazar_core
        PUBLIC "${PROJECT_SOURCE_DIR}/include"
)
