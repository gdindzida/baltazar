add_executable(baltazar_core_test core_test.cpp)
target_link_libraries(baltazar_core_test PUBLIC gtest_main PRIVATE baltazar_core_lib)
include(GoogleTest)

