add_library(baltazar_core core.cpp)

target_include_directories(baltazar_core
        PUBLIC "${PROJECT_SOURCE_DIR}/include"
)
