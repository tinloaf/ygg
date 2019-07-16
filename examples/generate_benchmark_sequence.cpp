// This must be defined before including any Ygg headers
#define YGG_STORE_SEQUENCE

#include "../src/ygg.hpp"

using namespace ygg;

class SequenceInterface {
public:
	using KeyT = int;

	// We use the template here to resolve the cycling dependency below
	template <class Node>
	static int
	get_key(const Node & n)
	{
		return n.key;
	}

	// This is necessary because we search by value
	static int
	get_key(int i)
	{
		return i;
	}
};

/* The tree options
 * We just supply the sequence interface
 */
using MyTreeOptions =
    TreeOptions<TreeFlags::BENCHMARK_SEQUENCE_INTERFACE<SequenceInterface>>;

class Node : public RBTreeNodeBase<Node, MyTreeOptions> {
public:
	int key;

	bool
	operator<(const Node & other) const
	{
		return this->key < other.key;
	}
};

// Configure the RBTree based on Node and the default NodeTraits
using MyTree = RBTree<Node, RBDefaultNodeTraits, MyTreeOptions>;

// We need this s.t. we can query by key value (i.e, an int) directly
bool
operator<(const Node & lhs, const int rhs)
{
	return lhs.key < rhs;
}
bool
operator<(const int lhs, const Node & rhs)
{
	return lhs < rhs.key;
}

int
main(int argc, char ** argv)
{
	(void)argc;
	(void)argv;

	MyTree t;

	// Storage for the actual nodes.
	// WARNING: using STL containers here can backfire badly. See TODO.
	Node nodes[5];

	// Initialize the nodes with some values
	for (int i = 0; i < 5; ++i) {
		nodes[i].key = i;
	}

	
	// Insert them
	for (size_t i = 0; i < 5; ++i) {
		t.insert(nodes[i]);
	}

	// Search for 3
	auto it = t.find(3);

	// Okay, we don't need that Node anymore.
	t.remove(*it);
	
	return 0;
}
