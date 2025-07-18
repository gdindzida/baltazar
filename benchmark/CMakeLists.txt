add_executable(baltazar_benchmark benchmark_example.cpp)
target_link_libraries(baltazar_benchmark benchmark::benchmark core_lib)

target_include_directories(baltazar_benchmark 
PUBLIC "${PROJECT_SOURCE_DIR}/include"
)
