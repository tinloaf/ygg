enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

find_package( Threads )

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -Og -fprofile-arcs -ftest-coverage" )
SET(CMAKE_LDFLAGS  "${CMAKE_LDFLAGS} -fprofile-arcs -ftest-coverage")

add_executable (run_tests main.cpp)
target_link_libraries (run_tests LINK_PUBLIC ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
