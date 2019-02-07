project(benchmark C CXX)

add_subdirectory(gbenchmark)

find_package(Boost REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(${GBENCHMARK_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

# Force optimization
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")

set(CMAKE_CXX_STANDARD 17)
set(LIBS "${LIBS} Threads::Threads ${GBENCHMARK_LIBS_DIR}/libbenchmark.a")

SET(BINARIES "bench_bst_insert;bench_bst_delete;bench_bst_search;bench_dst_insert;bench_dst_delete;bench_dst_move;run_all")

FOREACH(BINARY ${BINARIES})
	add_executable(${BINARY} ${BINARY}.cpp)
	add_dependencies(${BINARY} gbenchmark)
	target_link_libraries(${BINARY} Threads::Threads ${GBENCHMARK_LIBS_DIR}/libbenchmark.a)

	if (PAPI_FOUND)
		add_executable(${BINARY}_papi ${BINARY}.cpp)
		add_dependencies(${BINARY}_papi gbenchmark)
		target_link_libraries(${BINARY}_papi Threads::Threads ${GBENCHMARK_LIBS_DIR}/libbenchmark.a ${PAPI_LIBRARIES})
		set_target_properties (${BINARY}_papi PROPERTIES COMPILE_DEFINITIONS "USEPAPI")
	endif()
ENDFOREACH()
										
add_executable(pathlengths pathlengths.cpp)

