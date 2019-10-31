#ifndef YGG_TEST_DST_H
#define YGG_TEST_DST_H

#include "../src/ygg.hpp"
#include "randomizer.hpp"

#include <algorithm>
#include <boost/icl/interval_map.hpp>
#include <gtest/gtest.h>
#include <random>
#include <vector>

namespace ygg {
namespace testing {
namespace dynamic_segment_tree {

using namespace boost::icl;
using namespace ygg;

constexpr int DYNSEGTREE_TESTSIZE = 2000;
constexpr int DYNSEGTREE_COMPREHENSIVE_TESTSIZE = 500;
constexpr int DYNSEGTREE_DELETION_TESTSIZE = 100;
constexpr int DYNSEGTREE_DELETION_ITERATIONS = 10;

using MCombiner = MaxCombiner<int, int>;
using RMCombiner = RangedMaxCombiner<int, int>;
using Combiners = CombinerPack<int, int, RMCombiner, MCombiner>;

} // namespace dynamic_segment_tree
} // namespace testing
} // namespace ygg

#define __DST_BASENAME(NAME) RBTree_##NAME
#define __DST_BASESELECTOR UseDefaultRBTree
#define DYNSEGTREE_SEED 4

#include "test_dynamic_segment_tree_base.hpp"

#undef __DST_BASENAME
#define __DST_BASENAME(NAME) WBTree_##NAME
#undef __DST_BASESELECTOR
#define __DST_BASESELECTOR UseDefaultWBTree
#undef DYNSEGTREE_SEED
#define DYNSEGTREE_SEED 5

#include "test_dynamic_segment_tree_base.hpp"

#undef __DST_BASENAME
#define __DST_BASENAME(NAME) ZipTree_##NAME
#undef __DST_BASESELECTOR
#define __DST_BASESELECTOR UseDefaultZipTree
#undef DYNSEGTREE_SEED
#define DYNSEGTREE_SEED 6

#include "test_dynamic_segment_tree_base.hpp"

#endif
