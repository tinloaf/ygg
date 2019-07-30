project(benchmark C CXX)

add_subdirectory(gbenchmark)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(SYSTEM ${GBENCHMARK_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/draup) 
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/draup/draup.hpp")
	message( FATAL_ERROR "draup.hpp not found. Did you forget to run 'git submodule update --init'?" )
endif()

set(CMAKE_CXX_STANDARD 17)
set(LIBS "${LIBS} Threads::Threads ${GBENCHMARK_LIBS_DIR}/libbenchmark.a")

SET(BINARIES "bench_bst_insert;bench_bst_move;bench_bst_delete;bench_bst_search;bench_dst_insert;bench_dst_delete;bench_dst_move;run_all;bench_bst_erase")

FOREACH(BINARY ${BINARIES})
	add_executable(${BINARY} ${BINARY}.cpp random.cpp)
	add_dependencies(${BINARY} gbenchmark)
	target_link_libraries(${BINARY} Threads::Threads ${GBENCHMARK_LIBS_DIR}/libbenchmark.a)

	if (PAPI_FOUND)
		add_executable(${BINARY}_papi ${BINARY}.cpp random.cpp)
		add_dependencies(${BINARY}_papi gbenchmark)
		target_link_libraries(${BINARY}_papi Threads::Threads ${GBENCHMARK_LIBS_DIR}/libbenchmark.a ${PAPI_LIBRARIES})
		set_target_properties (${BINARY}_papi PROPERTIES COMPILE_DEFINITIONS "USEPAPI")
	endif()
ENDFOREACH()

SET(ZIPF_BINARIES "bench_bst_move;bench_bst_insert;bench_bst_search")
FOREACH(BINARY ${ZIPF_BINARIES})
	add_executable(${BINARY}_zipf ${BINARY}.cpp random.cpp)
	add_dependencies(${BINARY}_zipf gbenchmark)
	target_link_libraries(${BINARY}_zipf Threads::Threads ${GBENCHMARK_LIBS_DIR}/libbenchmark.a)
	set_target_properties (${BINARY}_zipf PROPERTIES COMPILE_DEFINITIONS "USEZIPF")
ENDFOREACH()

SET(SKEWED_BINARIES "bench_bst_move;bench_bst_insert;bench_bst_search")
FOREACH(BINARY ${SKEWED_BINARIES})
	add_executable(${BINARY}_skewed ${BINARY}.cpp random.cpp)
	add_dependencies(${BINARY}_skewed gbenchmark)
	target_link_libraries(${BINARY}_skewed Threads::Threads ${GBENCHMARK_LIBS_DIR}/libbenchmark.a)
	set_target_properties (${BINARY}_skewed PROPERTIES COMPILE_DEFINITIONS "USESKEWED")
ENDFOREACH()


add_executable(pathlengths pathlengths.cpp)
add_executable(wbtree_balance wbtree_balance.cpp)
add_executable(count_rotations count_rotations.cpp)
add_executable(translate_sequence translate_sequence.cpp)
add_executable(sequence sequence.cpp)
