add_library(baltazar_utils_lib INTERFACE)

target_include_directories(baltazar_utils_lib INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(test/)
