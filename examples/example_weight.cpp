//
// Created by lukas on 26.09.17.
//

#include "../src/ygg.hpp"

using namespace ygg;

/* The tree options
 * We allow multiple nodes with the same key to be inserted into the tree.
 */
using MyTreeOptions = TreeOptions<TreeFlags::MULTIPLE>;

/* The node class
 * Provides a simple key -> value mapping, where the key is an integer and the
 * value is a string.
 */
class Node : public weight::WBTreeNodeBase<Node, MyTreeOptions> {
public:
	int key;
	std::string value;

	// need to implement this s.t. we can use the default
	// ygg::utilities::flexible_less as comparator
	bool
	operator<(const Node & other) const
	{
		return this->key < other.key;
	}
};

// Configure the RBTree based on Node and the default NodeTraits
using MyTree = weight::WBTree<Node, RBDefaultNodeTraits, MyTreeOptions>;

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
		nodes[i].value = std::string("The key is ") + std::to_string(i);
	}

	// Insert them
	for (size_t i = 0; i < 5; ++i) {
		t.insert(nodes[i]);
	}

	// What was the string for i = 3 again?
	auto it = t.find(3); // Note we're using a int to query here, not a Node
	assert(it != t.end());
	std::string retrieved_value = it->value; // *it is the Node

	// Okay, we don't need that Node anymore.
	t.remove(*it);

	return 0;
}
