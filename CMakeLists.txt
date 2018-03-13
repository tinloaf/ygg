cmake_minimum_required(VERSION 3.2)
project (YGG)

#set (CMAKE_CXX_STANDARD 14)

# Set a default build type if none was specified
set(default_build_type "Debug")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
          STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
          "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

add_subdirectory(test)
add_subdirectory(examples)

if(NOT "${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" VERSION_LESS "3.8")
  find_library(CELERO_LIB_DIR libcelero.so)
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag(-std=c++17 HASCPP17)
  if (CELERO_LIB_DIR AND HASCPP17)
    add_subdirectory (benchmark)
  else()
    message(WARNING "Celero and a C++17 capable compiler are necessary to build benchmarks.")
  endif()
else()
  message(WARNING "CMake >= 3.8.0 required to build benchmarks.")
  message(INFO "You are running CMake version ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}")
endif()

# make CLion analyze all files
add_custom_target(clion_dummy SOURCES src/rbtree.hpp src/intervaltree.hpp src/ygg.hpp
        src/intervaltree.cpp src/rbtree.cpp src/ygg.hpp src/util.hpp src/options.hpp
        src/intervalmap.hpp src/intervalmap.cpp src/list.hpp src/list.cpp
        src/dynamic_segment_tree.cpp src/dynamic_segment_tree.hpp src/debug.hpp)
