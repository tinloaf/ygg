add_executable (example_rbtree example_rbtree.cpp)

set_target_properties(example_rbtree
                      PROPERTIES COMPILE_FLAGS "-g -O0 -Wall -Wextra -pedantic"
                      )

add_executable (example_intervaltree example_intervaltree.cpp)

set_target_properties(example_intervaltree
        PROPERTIES COMPILE_FLAGS "-g -O0 -Wall -Wextra -pedantic"
        )

add_executable (example_intervalmap example_intervalmap.cpp)

set_target_properties(example_intervalmap
        PROPERTIES COMPILE_FLAGS "-g -O0 -Wall -Wextra -pedantic"
        )