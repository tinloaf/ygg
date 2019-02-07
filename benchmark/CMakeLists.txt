project(benchmark C CXX)

add_subdirectory(gbenchmark)
add_subdirectory(draup)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(${GBENCHMARK_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/draup/src/draup) # TODO why is DRAUP_INCLUDE_DIRS not set?
include_directories(${Boost_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 17)
set(LIBS "${LIBS} Threads::Threads ${GBENCHMARK_LIBS_DIR}/libbenchmark.a")

SET(BINARIES "bench_bst_insert;bench_bst_delete;bench_bst_search;bench_dst_insert;bench_dst_delete;bench_dst_move;run_all")

FOREACH(BINARY ${BINARIES})
	add_executable(${BINARY} ${BINARY}.cpp)
	add_dependencies(${BINARY} gbenchmark)
	add_dependencies(${BINARY} draup)
	target_link_libraries(${BINARY} Threads::Threads ${GBENCHMARK_LIBS_DIR}/libbenchmark.a)

	if (PAPI_FOUND)
		add_executable(${BINARY}_papi ${BINARY}.cpp)
		add_dependencies(${BINARY}_papi gbenchmark)
		target_link_libraries(${BINARY}_papi Threads::Threads ${GBENCHMARK_LIBS_DIR}/libbenchmark.a ${PAPI_LIBRARIES})
		set_target_properties (${BINARY}_papi PROPERTIES COMPILE_DEFINITIONS "USEPAPI")
	endif()
ENDFOREACH()
										
add_executable(pathlengths pathlengths.cpp)

