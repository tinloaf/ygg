#ifndef YGG_TEST_RBT_H
#define YGG_TEST_RBT_H

#include "../src/ygg.hpp"
#include "randomizer.hpp"

#include <algorithm>
#include <gtest/gtest.h>
#include <random>
#include <vector>

namespace ygg {
namespace testing {
namespace rbtree {

constexpr int RBTREE_TESTSIZE = 2000;

struct NonOptionDummy;

template <class AdditionalOption = NonOptionDummy>
using BasicNonMultipleOptions =
    TreeOptions<TreeFlags::CONSTANT_TIME_SIZE, AdditionalOption>;
template <class AdditionalOption = NonOptionDummy>
using BasicMultipleOptions = TreeOptions<TreeFlags::CONSTANT_TIME_SIZE,
                                         TreeFlags::MULTIPLE, AdditionalOption>;

template <class AdditionalOption = NonOptionDummy>
using AdvancedNonMultipleOptions =
    TreeOptions<TreeFlags::CONSTANT_TIME_SIZE, TreeFlags::COMPRESS_COLOR,
                TreeFlags::MICRO_AVOID_CONDITIONALS, AdditionalOption>;
template <class AdditionalOption = NonOptionDummy>
using AdvancedMultipleOptions =
    TreeOptions<TreeFlags::CONSTANT_TIME_SIZE, TreeFlags::MULTIPLE,
                TreeFlags::COMPRESS_COLOR, TreeFlags::MICRO_AVOID_CONDITIONALS,
                AdditionalOption>;

#define __RBT_BASENAME(NAME) Basic_##NAME
#define __RBT_NONMULTIPLE BasicNonMultipleOptions
#define __RBT_MULTIPLE BasicMultipleOptions
// chosen by fair xkcd
#define RBTREE_SEED 4

namespace basic {
#include "test_rbtree_base.hpp"
}

#undef __RBT_BASENAME
#undef __RBT_NONMULTIPLE
#undef __RBT_MULTIPLE
#undef RBTREE_SEED
#define __RBT_BASENAME(NAME) Advanced_##NAME
#define __RBT_NONMULTIPLE AdvancedNonMultipleOptions
#define __RBT_MULTIPLE AdvancedMultipleOptions
#define RBTREE_SEED 5

namespace advanced {
#include "test_rbtree_base.hpp"
}

} // namespace rbtree
} // namespace testing
} // namespace ygg

#endif
