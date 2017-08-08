cmake_minimum_required (VERSION 2.8.11)
project (TREETEST)

macro(use_cxx11)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_CXX_FLAGS "--std=gnu++11 ${CMAKE_CXX_FLAGS}")
    endif ()
  else ()
    set (CMAKE_CXX_STANDARD 11)
  endif ()
endmacro(use_cxx11)

use_cxx11()

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra" )

add_subdirectory (test)
add_subdirectory (timing)

# make CLion analyze all files
add_custom_target(clion_dummy SOURCES src/rbtree.hpp src/intervaltree.hpp src/ygg.hpp
        src/intervaltree.cpp src/rbtree.cpp)
