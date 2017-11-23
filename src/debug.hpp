//
// Created by lukas on 20.11.17.
//

#ifndef YGG_DEBUG_HPP
#define YGG_DEBUG_HPP

namespace ygg {
namespace debug {

template<class Node>
Node * dbg_find_root(Node * n) {
	while (n->_rbt_parent != nullptr) {
		n = n->_rbt_parent;
	}

	return n;
}

template <class Node, class NodeNameGetter>
class TreePrinter
{
public:
	TreePrinter(Node *root_in, NodeNameGetter nng_in)
					: root(root_in), nng(nng_in)
	{}

	void reset_root(Node * new_root)
	{
		this->root = new_root;
	}

	void
	print() const
	{
		if (root == nullptr) {
			std::cout << "(Empty)\n";
		} else {
			std::vector<std::string> prefix;
			this->print_node(root, prefix, 0);
		}
	}

private:
	void
	print_node(Node *node, std::vector<std::string> & prefix, int direction) const
	{
		for (auto c : prefix) {
			std::cout << c;
		}

		if (direction == 0) {
			std::cout << "┌";
		} else if (direction < 0) {
			std::cout << "├";
			prefix.push_back(std::string("│"));
		} else {
			std::cout << "└";
			prefix.push_back(std::string(" "));
		}

		if (node == nullptr) {
			std::cout << "─ (empty)\n";
		} else {
			std::cout << "┬";

			std::cout << " " << nng.get_name(node);
			std::cout << "\n";


			this->print_node(node->_rbt_left, prefix, -1);
			this->print_node(node->_rbt_right, prefix, 1);
		}
		if (direction != 0) {
			prefix.pop_back();
		}
	}

	Node *root;
	NodeNameGetter nng;
};

}
}

#endif //YGG_DEBUG_HPP
