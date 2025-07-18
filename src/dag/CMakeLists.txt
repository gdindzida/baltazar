add_library(baltazar_dag_lib INTERFACE)

target_include_directories(baltazar_dag_lib INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(baltazar_dag_lib INTERFACE baltazar_thread_pool_lib baltazar_utils_lib)

add_subdirectory(test/)
