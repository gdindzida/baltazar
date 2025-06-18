add_library(core_lib
        core.h
        core.cpp
)

target_include_directories(core_lib
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_subdirectory(test/)
add_subdirectory(benchmark/)