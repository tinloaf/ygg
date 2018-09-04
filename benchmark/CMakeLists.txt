find_package(Boost REQUIRED)
find_path(CELERO_HEADERS celero/Celero.h)

include_directories(${CELERO_HEADERS} ${Boost_INCLUDE_DIRS})

add_executable (benchmark main.cpp)

target_link_libraries(benchmark ${CELERO_LIB_DIR})

set_target_properties(benchmark
                      PROPERTIES COMPILE_FLAGS "-O3 -march=native")
set_target_properties(benchmark
                      PROPERTIES CXX_STANDARD 17)
