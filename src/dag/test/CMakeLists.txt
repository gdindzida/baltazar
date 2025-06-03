add_executable(dag_test dag_test.cpp)
target_link_libraries(dag_test PUBLIC gtest_main PRIVATE thread_pool_lib)
include(GoogleTest)
