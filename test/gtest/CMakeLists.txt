# Taken from https://github.com/snikulov/google-test-examples/blob/master/ext/gtest/CMakeLists.txt
cmake_minimum_required(VERSION 2.8.8)
project(gtest_builder C CXX)
include(ExternalProject)

set(CMAKE_CXX_STANDARD 17)

set(GTEST_FORCE_SHARED_CRT ON)
set(GTEST_DISABLE_PTHREADS OFF)

if(MINGW)
    set(GTEST_DISABLE_PTHREADS ON)
endif()

ExternalProject_Add(googletest
  GIT_REPOSITORY       https://github.com/google/googletest.git
	GIT_TAG              b5fd99bbd55ebe1a3488b8ea3717fba089293457
  CMAKE_ARGS           -DCMAKE_CXX_FLAGS=${MSVC_COMPILER_DEFS}
                       -Dgtest_force_shared_crt=${GTEST_FORCE_SHARED_CRT}
											 -Dgtest_disable_pthreads=${GTEST_DISABLE_PTHREADS}
  PREFIX               "${CMAKE_CURRENT_BINARY_DIR}"
  # Disable install step
  INSTALL_COMMAND ""
  )

# Specify include dir
ExternalProject_Get_Property(googletest source_dir)
set(GTEST_INCLUDE_DIRS ${source_dir}/googletest/include PARENT_SCOPE)

# Specify MainTest's link libraries
ExternalProject_Get_Property(googletest binary_dir)
set(GTEST_LIBS_DIR ${binary_dir}/lib PARENT_SCOPE)
