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
struct EmptyDummyOpt;
namespace wbtree_twopass {

#define __WBT_BASENAME(NAME) TP_##NAME
#define __WBT_NAMESPACE wbtree_twopass
template <class AddOpt = EmptyDummyOpt>
using MULTI_FLAGS =
    TreeOptions<TreeFlags::MULTIPLE, TreeFlags::CONSTANT_TIME_SIZE, AddOpt>;
template <class AddOpt = EmptyDummyOpt>
using DEFAULT_FLAGS = TreeOptions<TreeFlags::CONSTANT_TIME_SIZE,
                                  TreeFlags::MICRO_AVOID_CONDITIONALS, AddOpt>;

#define WBTREE_SEED 4
#include "test_wbtree_base.hpp"

} // namespace wbtree_twopass

namespace wbtree_onepass {

#undef __WBT_BASENAME
#define __WBT_BASENAME(NAME) SP_##NAME
#undef __WBT_NAMESPACE
#define __WBT_NAMESPACE wbtree_onepass
template <class AddOpt = EmptyDummyOpt>
using MULTI_FLAGS = TreeOptions<TreeFlags::WBT_SINGLE_PASS, TreeFlags::MULTIPLE,
                                TreeFlags::CONSTANT_TIME_SIZE, AddOpt>;
template <class AddOpt = EmptyDummyOpt>
using DEFAULT_FLAGS =
    TreeOptions<TreeFlags::WBT_SINGLE_PASS, TreeFlags::CONSTANT_TIME_SIZE,
                TreeFlags::MICRO_AVOID_CONDITIONALS, AddOpt>;

#undef WBTREE_SEED
#define WBTREE_SEED 5
#include "test_wbtree_base.hpp"

} // namespace wbtree_onepass

namespace wbtree_smalldelta {

#undef __WBT_BASENAME
#define __WBT_BASENAME(NAME) SD_##NAME
#undef __WBT_NAMESPACE
#define __WBT_NAMESPACE wbtree_smalldelta
template <class AddOpt = EmptyDummyOpt>
using MULTI_FLAGS =
    TreeOptions<TreeFlags::WBT_SINGLE_PASS, TreeFlags::MULTIPLE,
                TreeFlags::CONSTANT_TIME_SIZE,
                ygg::TreeFlags::WBT_DELTA_NUMERATOR<3>,
                ygg::TreeFlags::WBT_DELTA_DENOMINATOR<2>,
                ygg::TreeFlags::WBT_GAMMA_NUMERATOR<5>,
                ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<4>, AddOpt>;

template <class AddOpt = EmptyDummyOpt>
using DEFAULT_FLAGS =
    TreeOptions<TreeFlags::WBT_SINGLE_PASS, TreeFlags::CONSTANT_TIME_SIZE,
                ygg::TreeFlags::WBT_DELTA_NUMERATOR<3>,
                ygg::TreeFlags::WBT_DELTA_DENOMINATOR<2>,
                ygg::TreeFlags::WBT_GAMMA_NUMERATOR<5>,
                ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<4>, AddOpt>;

#undef WBTREE_SEED
#define WBTREE_SEED 5
#include "test_wbtree_base.hpp"

} // namespace wbtree_smalldelta

} // namespace testing
} // namespace ygg

#endif
