add_library(dag_lib INTERFACE)

target_include_directories(dag_lib INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(dag_lib INTERFACE thread_pool_lib utils_lib)

add_subdirectory(test/)