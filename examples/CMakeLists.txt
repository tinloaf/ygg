add_executable (example_rbtree example_rbtree.cpp)
set_target_properties(example_rbtree
        PROPERTIES CXX_STANDARD 17)
set_target_properties(example_rbtree
                      PROPERTIES COMPILE_FLAGS "-g -O0 -Wall -Wextra -pedantic"
                      )

add_executable (example_energy example_energy.cpp)
set_target_properties(example_energy
        PROPERTIES CXX_STANDARD 17)
set_target_properties(example_energy
                      PROPERTIES COMPILE_FLAGS "-g -O0 -Wall -Wextra -pedantic"
                      )

add_executable (example_weight example_weight.cpp)
set_target_properties(example_weight
        PROPERTIES CXX_STANDARD 17)
set_target_properties(example_weight
                      PROPERTIES COMPILE_FLAGS "-g -O0 -Wall -Wextra -pedantic"
                      )										
										 
add_executable (example_ziptree example_ziptree.cpp)
set_target_properties(example_ziptree
        PROPERTIES CXX_STANDARD 17)
set_target_properties(example_ziptree
                      PROPERTIES COMPILE_FLAGS "-g -O0 -Wall -Wextra -pedantic"
                      )

add_executable (example_intervaltree example_intervaltree.cpp)
set_target_properties(example_intervaltree
        PROPERTIES CXX_STANDARD 17)
set_target_properties(example_intervaltree
        PROPERTIES COMPILE_FLAGS "-g -O0 -Wall -Wextra -pedantic"
        )

add_executable (example_intervalmap example_intervalmap.cpp)
set_target_properties(example_intervalmap
        PROPERTIES CXX_STANDARD 17)
set_target_properties(example_intervalmap
        PROPERTIES COMPILE_FLAGS "-g -O0 -Wall -Wextra -pedantic"
        )

add_executable (example_dynamic_segment_tree example_dynamic_segment_tree.cpp)
set_target_properties(example_dynamic_segment_tree
        PROPERTIES CXX_STANDARD 17)
set_target_properties(example_dynamic_segment_tree
        PROPERTIES COMPILE_FLAGS "-g -O0 -Wall -Wextra -pedantic"
        )

add_executable (generate_benchmark_sequence generate_benchmark_sequence.cpp)
set_target_properties(generate_benchmark_sequence
        PROPERTIES CXX_STANDARD 17)
set_target_properties(generate_benchmark_sequence
        PROPERTIES COMPILE_FLAGS "-g -O0 -Wall -Wextra -pedantic"
        )
