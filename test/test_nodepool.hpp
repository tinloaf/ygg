#ifndef YGG_TEST_NODEPOOL_HPP
#define YGG_TEST_NODEPOOL_HPP

#include "../src/nodepool.hpp"

namespace ygg {
namespace testing {
namespace nodepool {

constexpr size_t TESTSIZE = 100;

using MyTreeOptions = TreeOptions<TreeFlags::MULTIPLE>;

class Node : public RBTreeNodeBase<Node, MyTreeOptions> {
public:
	size_t key;

	bool
	operator<(const Node & other) const
	{
		return this->key < other.key;
	}
};

using MyTree = RBTree<Node, RBDefaultNodeTraits, MyTreeOptions>;
using MyNodePool = bst::nodepool::NodePool<MyTree>;

TEST(NodePoolTest, InsertAndRemoveTest)
{
	MyTree t;
	MyNodePool np;
	Node nodes[TESTSIZE];

	for (size_t i = 0; i < TESTSIZE; ++i) {
		nodes[i].key = i;
		t.insert(nodes[i]);
	}

	// Remove and insert into node pool
	for (size_t i = 0; i < TESTSIZE; ++i) {
		t.remove(nodes[i]);
		np.insert(&nodes[i]);
	}

	for (size_t i = 0; i < TESTSIZE; ++i) {
		// See if we get them out in the correct order
		auto n = np.get();
		ASSERT_EQ(n, &nodes[TESTSIZE - 1 - i]);
		t.insert(*n);
		t.dbg_verify();
	}
}

} // namespace nodepool
} // namespace testing
} // namespace ygg

#endif
