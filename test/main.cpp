#include <gtest/gtest.h>

#include "test_dynamic_segment_tree.hpp"
#include "test_intervaltree.hpp"
#include "test_list.hpp"
#include "test_multi_rbtree.hpp"
#include "test_rbtree.hpp"
#include "test_ziptree.hpp"
#include "test_energy.hpp"
#include "test_wbtree.hpp"
#include "test_nodepool.hpp"

int
main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
