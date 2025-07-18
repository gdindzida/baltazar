add_library(baltazar_thread_pool_lib INTERFACE)

target_include_directories(baltazar_thread_pool_lib INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(test/)
add_subdirectory(benchmark/)
