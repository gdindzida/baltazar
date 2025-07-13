add_library(core_lib INTERFACE)

target_include_directories(core_lib INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(test/)
add_subdirectory(benchmark/)