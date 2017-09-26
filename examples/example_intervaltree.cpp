//
// Created by lukas on 26.09.17.
//

#include "../src/ygg.hpp"

using namespace ygg;

/* We need do derive our own NodeTraits. This class will be used by the IntervalTree to retrieve
 * the upper and lower interval borders from the nodes.
 */
template<class Node>
class NodeTraits : public ITreeNodeTraits<Node> {
public:
	using key_type = int;

	static int get_lower(const Node & node) {
		return node.lower;
	}

	static int get_upper(const Node & node) {
		return node.upper;
	}
};

/* This is the node class. It provides a simple interval -> value mapping, with the interval
 * borders being integers and the value being a string.
 */
class Node : public ITreeNodeBase<Node, NodeTraits<Node>> {
public:
	int upper;
	int lower;

	std::string value;

	// No need to implement operator< here. Intervals are compared based on what
	// get_upper() and get_lower() return.
};

// Configure the IntervalTree
using MyTree = IntervalTree<Node, NodeTraits<Node>>;

int main(int argc, char **argv) {
	(void)argc;
	(void)argv;

	MyTree t;

	// Storage for the actual nodes.
	// WARNING: using STL containers here can backfire badly. See TODO.
	Node nodes[5];

	// Initialize the nodes with some values
	for (size_t i = 0 ; i < 5 ; ++i) {
		nodes[i].lower = i;
		nodes[i].upper = i + 10;
		nodes[i].value = std::string("The interval is [") + std::to_string(i) + std::string("]");
	}

	// Insert them
	for (size_t i = 0 ; i < 5 ; ++i) {
		t.insert(nodes[i]);
	}

	// Querying explained below

	// Delete one node
	t.remove(nodes[3]);
}