find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
find_package( Threads )

# Find Boost
find_package(Boost REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Boost_LIBRARIES})

add_executable (run_tests main.cpp)

set(CMAKE_C_FLAGS_DEBUG "-fno-omit-frame-pointer -Wall -Wextra -pedantic -Wfloat-equal \
-Wundef -Wshadow -Wpointer-arith -Wcast-align -Wstrict-overflow=5 -Wwrite-strings -Wcast-qual \
-Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code -Wfloat-equal -Wswitch-default")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")

set_target_properties(run_tests
                      PROPERTIES COMPILE_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage \
                      -Wall -Wextra -pedantic"
                      )
set_target_properties(run_tests
                      PROPERTIES LINK_FLAGS "-fprofile-arcs -ftest-coverage"
                      )
set_target_properties(run_tests
                      PROPERTIES CXX_STANDARD 14
                      )

target_link_libraries (run_tests LINK_PUBLIC ${LIBS} ${GTEST_BOTH_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT} gcov)

# make CLion analyze all files
add_custom_target(clion_test_dummy SOURCES test_intervaltree.hpp
    test_rbtree.hpp test_list.hpp test_multi_rbtree.hpp test_intervalmap.hpp test_dynamic_segment_tree.hpp)

enable_testing()
add_test(NAME gtest COMMAND run_tests)

# copy gdb pretty printers to build dir
add_custom_command(TARGET run_tests PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/gdbscripts ${CMAKE_BINARY_DIR}/test/gdbscripts)