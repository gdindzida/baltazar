add_executable(baltazar_test test_example.cpp)
target_link_libraries(baltazar_test gtest_main baltazar_core)
include(GoogleTest)
gtest_discover_tests(baltazar_test)

target_include_directories(baltazar_test 
PUBLIC "${PROJECT_SOURCE_DIR}/include/"
)