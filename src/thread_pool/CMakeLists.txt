add_library(thread_pool_lib
        thread_pool.h
        thread_task.h
        thread_task_queue.h
        thread_pool.cpp
)

target_include_directories(thread_pool_lib
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_subdirectory(test/)
add_subdirectory(benchmark/)