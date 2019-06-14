#ifndef YGG_TEST_WBTREE_HPP
#define YGG_TEST_WBTREE_HPP

#include "../src/wbtree.hpp"
#include "randomizer.hpp"

#include <algorithm>
#include <gtest/gtest.h>
#include <random>
#include <sstream>
#include <vector>

namespace ygg {
namespace testing {
namespace wbtree_twopass {

#define __WBT_BASENAME(NAME) TP_##NAME
#define __WBT_NAMESPACE wbtree_twopass
using MULTI_FLAGS =
    TreeOptions<TreeFlags::MULTIPLE, TreeFlags::CONSTANT_TIME_SIZE>;
using DEFAULT_FLAGS = TreeOptions<TreeFlags::CONSTANT_TIME_SIZE,
                                  TreeFlags::MICRO_AVOID_CONDITIONALS,
                                  TreeFlags::MICRO_AVOID_CONDITIONALS_SETTING,
                                  TreeFlags::MICRO_DUMMY_SETTING_POINTER>;

#include "test_wbtree_base.hpp"

} // namespace wbtree_twopass

namespace wbtree_onepass {

#undef __WBT_BASENAME
#define __WBT_BASENAME(NAME) SP_##NAME
#undef __WBT_NAMESPACE
#define __WBT_NAMESPACE wbtree_onepass
using MULTI_FLAGS = TreeOptions<TreeFlags::WBT_SINGLE_PASS, TreeFlags::MULTIPLE,
                                TreeFlags::CONSTANT_TIME_SIZE>;
using DEFAULT_FLAGS =
    TreeOptions<TreeFlags::WBT_SINGLE_PASS, TreeFlags::CONSTANT_TIME_SIZE,
                TreeFlags::MICRO_AVOID_CONDITIONALS,
                TreeFlags::MICRO_AVOID_CONDITIONALS_SETTING,
                TreeFlags::MICRO_DUMMY_SETTING_POINTER>;

#include "test_wbtree_base.hpp"

} // namespace wbtree_onepass

} // namespace testing
} // namespace ygg

#endif
