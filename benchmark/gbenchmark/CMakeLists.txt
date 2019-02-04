cmake_minimum_required(VERSION 2.8.8)
project(gbenchmark_builder C CXX)
include(ExternalProject)

set(CMAKE_CXX_STANDARD 11)

ExternalProject_Add(gbenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        CMAKE_ARGS -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG:PATH=DebugLibs
        -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE:PATH=ReleaseLibs
				-DBENCHMARK_ENABLE_GTEST_TESTS=OFF
        PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
        # Disable install step
        INSTALL_COMMAND ""
        )

# Specify include dir
ExternalProject_Get_Property(gbenchmark source_dir)
set(GBENCHMARK_INCLUDE_DIRS ${source_dir}/include/benchmark PARENT_SCOPE)

# Specify link libraries
ExternalProject_Get_Property(gbenchmark binary_dir)
set(GBENCHMARK_LIBS_DIR ${binary_dir}/src PARENT_SCOPE)
