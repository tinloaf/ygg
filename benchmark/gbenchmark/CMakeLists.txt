cmake_minimum_required(VERSION 3.0.2)
project(gbenchmark_builder C CXX)
include(ExternalProject)

set(CMAKE_CXX_STANDARD 11)

ExternalProject_Add(gbenchmark
  GIT_REPOSITORY    https://github.com/google/benchmark.git
	GIT_TAG           b8ca0c42179b7b5d656494e61dda8b861057122f
  CMAKE_ARGS       -DBENCHMARK_ENABLE_GTEST_TESTS=OFF
	                 -DCMAKE_BUILD_TYPE=Release
	PREFIX           "${CMAKE_CURRENT_BINARY_DIR}"
  # Disable install step
  INSTALL_COMMAND ""
  )

# Specify include dir
ExternalProject_Get_Property(gbenchmark source_dir)
set(GBENCHMARK_INCLUDE_DIRS ${source_dir}/include/benchmark PARENT_SCOPE)

# Specify link libraries
ExternalProject_Get_Property(gbenchmark binary_dir)
set(GBENCHMARK_LIBS_DIR ${binary_dir}/src PARENT_SCOPE)
