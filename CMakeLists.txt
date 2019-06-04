cmake_minimum_required(VERSION 3.8)
project (YGG)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

set (CMAKE_CXX_STANDARD 17)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
         "Valgrind")

# Work around Boost bug: https://github.com/boostorg/icl/issues/21
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "7.0")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-new-ttp-matching")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-new-ttp-matching")
	endif()
endif()

#===============================================
#==== Build type: Release
#===============================================
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -ffast-math -march=native -flto")


#===============================================
#==== Build type: Debug
#===============================================
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -pedantic -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wstrict-overflow=5 -Wwrite-strings -Wcast-qual -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code -Wfloat-equal -Wold-style-cast -Wnull-dereference")
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "7.0")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wuseless-cast -Wlogical-op -Wduplicated-branches -Wduplicated-cond")
  endif()
endif()


#===============================================
#==== Build type: valgrind
#===============================================

set(CMAKE_C_FLAGS_VALGRIND "${CMAKE_C_FLAGS} -O2 -march=native -ffast-math -g -flto" CACHE STRING
        "Flags used by the C++ compiler during valgrind builds."
        FORCE)
set(CMAKE_CXX_FLAGS_VALGRIND "${CMAKE_C_FLAGS_VALGRIND}" CACHE STRING
        "Flags used by the C compiler during valgrind builds."
        FORCE )
set (CMAKE_SHARED_LINKER_FLAGS_VALGRIND "${CMAKE_SHARED_LINKER_FLAGS}")
set (CMAKE_EXE_LINKER_FLAGS_VALGRIND "${CMAKE_EXE_LINKER_FLAGS}")
MARK_AS_ADVANCED(
        CMAKE_CXX_FLAGS_VALGRIND
        CMAKE_C_FLAGS_VALGRIND
        CMAKE_EXE_LINKER_FLAGS_VALGRIND
        CMAKE_SHARED_LINKER_FLAGS_VALGRIND )

# Set a default build type if none was specified
set(default_build_type "Debug")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
          STRING "Choose the type of build." FORCE)
endif()

add_subdirectory(examples)
add_subdirectory(test)


find_package(Boost 1.61)

if (Boost_FOUND)

	# Find PAPI
	find_package(PAPI)
	if (PAPI_FOUND)
		include_directories(SYSTEM ${PAPI_INCLUDE_DIRS})
		set(LIBS ${LIBS} ${PAPI_LIBRARIES})
		MESSAGE(STATUS, "Enabling statistics collection via PAPI.")
	endif(PAPI_FOUND)

	add_subdirectory (benchmark)
else()
	MESSAGE(WARNING, "Not building benchmarks or tests, Boost >= 1.64 not found.")
endif()
