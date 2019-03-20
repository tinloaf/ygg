#include <gtest/gtest.h>

#include "test_dynamic_segment_tree.hpp"
#include "test_intervalmap.hpp"
#include "test_intervaltree.hpp"
#include "test_list.hpp"
#include "test_multi_rbtree.hpp"
#include "test_rbtree.hpp"
#include "test_ziptree.hpp"
#include "test_energy.hpp"

//#include "test_orderlist.hpp"

/* Note: The "MS" section flags are to remove duplicates.  */
#define DEFINE_GDB_PY_SCRIPT(script_name)                                      \
  asm("\
.pushsection \".debug_gdb_scripts\", \"MS\",@progbits,1\n\
.byte 1 /* Python */\n\
.asciz \"" script_name "\"\n\
.popsection \n\
");

DEFINE_GDB_PY_SCRIPT("gdbscripts/ygg_printers.py")

int
main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
