include_directories(SYSTEM ${CELERO_INCLUDE_DIRS})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Boost_LIBRARIES} ${CELERO_LIBRARIES})

if (PAPI_FOUND)
	include_directories(SYSTEM ${PAPI_INCLUDE_DIRS})
	set(LIBS ${LIBS} ${PAPI_LIBRARIES})
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in"
        "${CMAKE_CURRENT_BINARY_DIR}/config.hpp" @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable (benchmark_rbtree main_rbtree.cpp)

target_link_libraries(benchmark_rbtree ${LIBS})

set_target_properties(benchmark_rbtree
                      PROPERTIES COMPILE_FLAGS "-O3 -march=native")
set_target_properties(benchmark_rbtree
                      PROPERTIES CXX_STANDARD 17)

add_executable(pathlengths pathlengths.cpp)
set_target_properties(pathlengths PROPERTIES CXX_STANDARD 17)

add_executable (benchmark_dst main_dst.cpp)

target_link_libraries(benchmark_dst ${LIBS})

set_target_properties(benchmark_dst
                      PROPERTIES COMPILE_FLAGS "-O3 -march=native")
set_target_properties(benchmark_dst
                      PROPERTIES CXX_STANDARD 17)

										
