add_library(dag_lib
        dag.h
        dag.cpp
)

target_link_libraries(dag_lib PRIVATE thread_pool_lib utils_lib)

target_include_directories(dag_lib
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_subdirectory(test/)