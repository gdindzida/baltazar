add_executable(core_test core_test.cpp)
target_link_libraries(core_test PUBLIC gtest_main PRIVATE core_lib)
include(GoogleTest)
