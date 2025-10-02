add_executable(baltazar_simple_example simple.cpp)

target_include_directories(baltazar_simple_example 
PUBLIC "${PROJECT_SOURCE_DIR}/include"
)

add_executable(baltazar_parallel_example parallel.cpp)

target_include_directories(baltazar_parallel_example 
PUBLIC "${PROJECT_SOURCE_DIR}/include"
)

add_executable(baltazar_profiling_example profiling.cpp)

target_include_directories(baltazar_profiling_example 
PUBLIC "${PROJECT_SOURCE_DIR}/include"
)