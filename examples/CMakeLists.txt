add_executable(baltazar_example example.cpp)
target_link_libraries(baltazar_example baltazar_core)

target_include_directories(baltazar_example 
PUBLIC "${PROJECT_SOURCE_DIR}/include/"
)