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

# Normal
add_executable(run_all run_all.cpp random.cpp)
add_dependencies(run_all gbenchmark)
target_link_libraries(run_all Threads::Threads ${GBENCHMARK_LIBS_DIR}/libbenchmark.a)

if (PAPI_FOUND)
	add_executable(run_all_papi run_all.cpp random.cpp)
	add_dependencies(run_all_papi gbenchmark)
	target_link_libraries(run_all_papi Threads::Threads ${GBENCHMARK_LIBS_DIR}/libbenchmark.a ${PAPI_LIBRARIES})
	set_target_properties (run_all_papi PROPERTIES COMPILE_DEFINITIONS "USEPAPI")
endif()

# Zipf
add_executable(run_all_zipf run_all.cpp random.cpp)
add_dependencies(run_all_zipf gbenchmark)
target_link_libraries(run_all_zipf Threads::Threads ${GBENCHMARK_LIBS_DIR}/libbenchmark.a)
set_target_properties (run_all_zipf PROPERTIES COMPILE_DEFINITIONS "USEZIPF")

if (PAPI_FOUND)
	add_executable(run_all_zipf_papi run_all.cpp random.cpp)
	add_dependencies(run_all_zipf_papi gbenchmark)
	target_link_libraries(run_all_zipf_papi Threads::Threads ${GBENCHMARK_LIBS_DIR}/libbenchmark.a ${PAPI_LIBRARIES})
	target_compile_definitions(run_all_zipf_papi PUBLIC USEPAPI USEZIPF)
endif()

# Presorted
add_executable(run_all_presorted run_all.cpp random.cpp)
add_dependencies(run_all_presorted gbenchmark)
target_link_libraries(run_all_presorted Threads::Threads ${GBENCHMARK_LIBS_DIR}/libbenchmark.a)
set_target_properties (run_all_presorted PROPERTIES COMPILE_DEFINITIONS "PRESORT")

if (PAPI_FOUND)
	add_executable(run_all_presorted_papi run_all.cpp random.cpp)
	add_dependencies(run_all_presorted_papi gbenchmark)
	target_link_libraries(run_all_presorted_papi Threads::Threads ${GBENCHMARK_LIBS_DIR}/libbenchmark.a ${PAPI_LIBRARIES})
	target_compile_definitions(run_all_presorted_papi PUBLIC USEPAPI PRESORT)
endif()

# Skewed
add_executable(run_all_skewed run_all.cpp random.cpp)
add_dependencies(run_all_skewed gbenchmark)
target_link_libraries(run_all_skewed Threads::Threads ${GBENCHMARK_LIBS_DIR}/libbenchmark.a)
set_target_properties (run_all_skewed PROPERTIES COMPILE_DEFINITIONS "USESKEWED")

if (PAPI_FOUND)
	add_executable(run_all_skewed_papi run_all.cpp random.cpp)
	add_dependencies(run_all_skewed_papi gbenchmark)
	target_link_libraries(run_all_skewed_papi Threads::Threads ${GBENCHMARK_LIBS_DIR}/libbenchmark.a ${PAPI_LIBRARIES})
	target_compile_definitions(run_all_skewed_papi PUBLIC USEPAPI USESKEWED)
endif()


# Create BST scripts
set(BENCH_DATASTRUCTURE "BST")
set(BST_OPERATIONS "Insert;Delete;Move;Erase")
FOREACH(OPERATION ${BST_OPERATIONS})
	FOREACH(POSTFIX "" _zipf _skewed _presorted)
		configure_file(${PROJECT_SOURCE_DIR}/../scripts/benchmark/benchmark.sh.in ${CMAKE_CURRENT_BINARY_DIR}/bench_${BENCH_DATASTRUCTURE}_${OPERATION}${POSTFIX})
		if (PAPI_FOUND)
			configure_file(${PROJECT_SOURCE_DIR}/../scripts/benchmark/benchmark_papi.sh.in ${CMAKE_CURRENT_BINARY_DIR}/bench_${BENCH_DATASTRUCTURE}_${OPERATION}${POSTFIX}_papi)
		endif()
	ENDFOREACH()
ENDFOREACH()

# Create DST scripts
set(BENCH_DATASTRUCTURE "DST")
set(DST_OPERATIONS "Insert;Delete;Move")
FOREACH(OPERATION ${DST_OPERATIONS})
	FOREACH(POSTFIX "" _zipf _skewed _presorted)
		configure_file(${PROJECT_SOURCE_DIR}/../scripts/benchmark/benchmark.sh.in ${CMAKE_CURRENT_BINARY_DIR}/bench_${BENCH_DATASTRUCTURE}_${OPERATION}${POSTFIX})
		if (PAPI_FOUND)
		configure_file(${PROJECT_SOURCE_DIR}/../scripts/benchmark/benchmark_papi.sh.in ${CMAKE_CURRENT_BINARY_DIR}/bench_${BENCH_DATASTRUCTURE}_${OPERATION}${POSTFIX}_papi)			
		endif()
	ENDFOREACH()
ENDFOREACH()


add_executable(pathlengths pathlengths.cpp random.cpp)
add_executable(wbtree_balance wbtree_balance.cpp)
add_executable(count_rotations count_rotations.cpp)
add_executable(translate_sequence translate_sequence.cpp)
add_executable(sequence sequence.cpp)

add_executable(paired paired.cpp random.cpp)
add_dependencies(paired gbenchmark)
target_link_libraries(paired Threads::Threads ${GBENCHMARK_LIBS_DIR}/libbenchmark.a)
