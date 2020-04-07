#include "../src/ygg.hpp"

#include <fstream>

/*
 * This example demonstrates how to use BST::dump_to_dot to visualize your tree
 * for debugging purposes. The BST::dump_to_dot method writes a .dot file to
 * a specified location, which you can then later visualize using the dot
 * program, which is part of Graphviz.
 */

using namespace ygg;

/* Standard RB-Tree setup. See example_rbtree.cpp for details.
 */
using MyTreeOptions = TreeOptions<TreeFlags::MULTIPLE>;

class Node : public RBTreeNodeBase<Node, MyTreeOptions> {
public:
	int key;
	std::string value;

	bool
	operator<(const Node & other) const
	{
		return this->key < other.key;
	}
};

/* In the node traits, we need to implement the static function
 * 'get_id(const Node *)'. It can return anything that can be
 * streamed into a std::stringstream. It should return a name
 * for every node - the nodes in the dot drawing will be labeled with
 * this.
 */
class NodeTraits : public RBDefaultNodeTraits {
public:
	static std::string
	get_id(const Node * n) noexcept
	{
		std::stringstream s;
		s << std::hex << reinterpret_cast<size_t>(n) << std::dec;
		return s.str();
	};
};

using MyTree = RBTree<Node, NodeTraits, MyTreeOptions>;

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
	Node nodes[5];
	for (int i = 0; i < 5; ++i) {
		nodes[i].key = i;
		nodes[i].value = std::string("The key is ") + std::to_string(i);
		t.insert(nodes[i]);
	}

	// We create the dot file at /tmp/my_tree.dot
	t.dump_to_dot<NodeTraits>("/tmp/my_tree.dot");

	// After this, you can get a PDF of your tree using the command:
	//   dot -Tpdf /tmp/my_tree.pdf /tmp/my_tree.dot

	return 0;
}
