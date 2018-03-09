cmake_minimum_required(VERSION 3.1)
project (YGG)

set (CMAKE_CXX_STANDARD 14)

set(CMAKE_C_FLAGS_DEBUG "-g -O0 -fno-omit-frame-pointer -Wall -Wextra -pedantic -Wfloat-equal \
-Wundef -Wshadow -Wpointer-arith -Wcast-align -Wstrict-overflow=5 -Wwrite-strings -Wcast-qual \
-Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code -Wfloat-equal -Wswitch-default")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")

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

find_library(CELERO_LIB_DIR libcelero.so)
if (CELERO_LIB_DIR)
  add_subdirectory (benchmark)
endif()

# make CLion analyze all files
add_custom_target(clion_dummy SOURCES src/rbtree.hpp src/intervaltree.hpp src/ygg.hpp
        src/intervaltree.cpp src/rbtree.cpp src/ygg.hpp src/util.hpp src/options.hpp
        src/intervalmap.hpp src/intervalmap.cpp src/list.hpp src/list.cpp
        src/dynamic_segment_tree.cpp src/dynamic_segment_tree.hpp src/debug.hpp)
