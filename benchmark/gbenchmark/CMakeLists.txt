cmake_minimum_required(VERSION 3.0.2)
project(gbenchmark_builder C CXX)
include(ExternalProject)

set(CMAKE_CXX_STANDARD 11)

ExternalProject_Add(gbenchmark
  GIT_REPOSITORY    https://github.com/google/benchmark.git
	GIT_TAG           b177433f3ee2513b1075140c723d73ab8901790f
  CMAKE_ARGS       -DBENCHMARK_ENABLE_GTEST_TESTS=OFF
	                 -DCMAKE_BUILD_TYPE=Release
	PREFIX           "${CMAKE_CURRENT_BINARY_DIR}"
  # Disable install step
  INSTALL_COMMAND ""
  )

# Specify include dir
ExternalProject_Get_Property(gbenchmark source_dir)
set(GBENCHMARK_INCLUDE_DIRS ${source_dir} PARENT_SCOPE)

# Specify link libraries
ExternalProject_Get_Property(gbenchmark binary_dir)
set(GBENCHMARK_LIBS_DIR ${binary_dir}/src PARENT_SCOPE)
