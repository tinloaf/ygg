project(benchmark C CXX)

add_subdirectory(gbenchmark)

find_package(Boost REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories("google_benchmark/include/benchmark")
include_directories(${Boost_INCLUDE_DIRS})

add_executable(benchmark_bst_insert bench_bst_insert.cpp)
add_dependencies(benchmark_bst_insert gbenchmark)
target_link_libraries(benchmark_bst_insert Threads::Threads  ${GBENCHMARK_LIBS_DIR}/libbenchmark.a)
set_target_properties(benchmark_bst_insert
                      PROPERTIES COMPILE_FLAGS "-O3 -march=native")
set_target_properties(benchmark_bst_insert
                      PROPERTIES CXX_STANDARD 17)


add_executable(benchmark_bst_delete bench_bst_delete.cpp)
add_dependencies(benchmark_bst_delete gbenchmark)
target_link_libraries(benchmark_bst_delete Threads::Threads  ${GBENCHMARK_LIBS_DIR}/libbenchmark.a)
set_target_properties(benchmark_bst_delete
                      PROPERTIES COMPILE_FLAGS "-O3 -march=native")
set_target_properties(benchmark_bst_delete
                      PROPERTIES CXX_STANDARD 17)

										
add_executable(benchmark_bst_search bench_bst_search.cpp)
add_dependencies(benchmark_bst_search gbenchmark)
target_link_libraries(benchmark_bst_search Threads::Threads  ${GBENCHMARK_LIBS_DIR}/libbenchmark.a)
set_target_properties(benchmark_bst_search
                      PROPERTIES COMPILE_FLAGS "-O3 -march=native")
set_target_properties(benchmark_bst_search
                      PROPERTIES CXX_STANDARD 17)


										
add_executable(pathlengths pathlengths.cpp)
set_target_properties(pathlengths PROPERTIES CXX_STANDARD 17)

# add_executable (benchmark_dst main_dst.cpp)

# target_link_libraries(benchmark_dst ${GBENCHMARK_LIBS_DIR}/benchmark.so)

# set_target_properties(benchmark_dst
#                       PROPERTIES COMPILE_FLAGS "-O3 -march=native")
# set_target_properties(benchmark_dst
#                       PROPERTIES CXX_STANDARD 17)
