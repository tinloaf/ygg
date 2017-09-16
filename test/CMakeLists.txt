cmake_minimum_required(VERSION 3.8)
enable_testing()
set (CMAKE_CXX_STANDARD 11)
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
find_package( Threads )

add_executable (run_tests main.cpp)

set_target_properties(run_tests
                      PROPERTIES COMPILE_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage"
                      )
set_target_properties(run_tests
                      PROPERTIES LINK_FLAGS "-fprofile-arcs -ftest-coverage"
                      )

target_link_libraries (run_tests LINK_PUBLIC ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

# make CLion analyze all files
add_custom_target(clion_test_dummy SOURCES test_intervaltree.hpp test_orderlist.hpp test_rbtree.hpp)
