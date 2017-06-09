#include <gtest/gtest.h>

#include "test_rbtree.hpp"
#include "test_intervaltree.hpp"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
