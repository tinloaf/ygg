find_package(Boost REQUIRED)
find_path(CELERO_HEADERS celero/Celero.h)

include_directories(${CELERO_HEADERS} ${Boost_INCLUDE_DIRS})

add_executable (benchmark_rbtree main_rbtree.cpp)

target_link_libraries(benchmark_rbtree ${CELERO_LIB_DIR})

set_target_properties(benchmark_rbtree
                      PROPERTIES COMPILE_FLAGS "-O3 -march=native")
set_target_properties(benchmark_rbtree
                      PROPERTIES CXX_STANDARD 17)

add_executable(pathlengths pathlengths.cpp)
set_target_properties(pathlengths PROPERTIES CXX_STANDARD 17)

add_executable (benchmark_dst main_dst.cpp)

target_link_libraries(benchmark_dst ${CELERO_LIB_DIR})

set_target_properties(benchmark_dst
                      PROPERTIES COMPILE_FLAGS "-O3 -march=native")
set_target_properties(benchmark_dst
                      PROPERTIES CXX_STANDARD 17)
