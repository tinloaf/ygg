add_executable (benchmark main.cpp)

add_executable (profile_insertion profile_insertion.cpp)
add_executable (profile_query profile_query.cpp)

set_target_properties(benchmark
                      PROPERTIES COMPILE_FLAGS "-O3")
set_target_properties(profile_insertion profile_query
                      PROPERTIES COMPILE_FLAGS "-O3 -pg -g"
                      )
set_target_properties(profile_insertion profile_query
                      PROPERTIES LINK_FLAGS "-pg"
                      )
