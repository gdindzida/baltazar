add_executable(core_benchmark core_benchmark.cpp)
target_link_libraries(core_benchmark PUBLIC benchmark::benchmark PRIVATE core_lib)

