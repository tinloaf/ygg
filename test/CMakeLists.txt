add_subdirectory(gtest)

find_package( Threads )

# Find Boost
find_package(Boost 1.58.0 REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Boost_LIBRARIES})

include_directories(SYSTEM ${GTEST_INCLUDE_DIRS})

add_executable (run_tests main.cpp)
add_dependencies(run_tests googletest)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -fno-omit-frame-pointer -Wall -Wextra -pedantic -Wfloat-equal \
-Wundef -Wshadow -Wpointer-arith -Wcast-align -Wstrict-overflow=5 -Wwrite-strings -Wcast-qual \
-Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code -Wfloat-equal -Wswitch-default")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")

set_target_properties(run_tests
                      PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage \
                      -Wall -Wextra -pedantic"
                      )
set_target_properties(run_tests
                      PROPERTIES LINK_FLAGS "-fprofile-arcs -ftest-coverage"
                      )
set_target_properties(run_tests
                      PROPERTIES CXX_STANDARD 17
                      )

target_link_libraries (run_tests ${LIBS} ${GTEST_LIBS_DIR}/libgtest.a ${GTEST_LIBS_DIR}/libgtest_main.a ${CMAKE_THREAD_LIBS_INIT})

string( TOLOWER "${CMAKE_BUILD_TYPE}" build_type_lower )
IF(build_type_lower MATCHES "coverage")
	target_link_libraries (run_tests gcov)
ENDIF()

# copy gdb pretty printers to build dir
add_custom_command(TARGET run_tests PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/gdbscripts ${CMAKE_BINARY_DIR}/test/gdbscripts)
