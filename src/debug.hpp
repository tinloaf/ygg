//
// Created by lukas on 20.11.17.
//

#ifndef YGG_DEBUG_HPP
#define YGG_DEBUG_HPP

template<class Tree, class Node, class NodeNameGetter>
class TreePrinter
{
public:
	TreePrinter(Tree * tree_in, NodeNameGetter nng_in) : tree(tree_in), nng(nng_in) {}

	void print() const
	{
		Node * root = this->tree->get_root();
		if (root == nullptr) {
			std::cout << "(Empty)\n";
		} else {
			this->print_node(root, 0, 0);
		}
	}
private:
	void print_node(Node * node, unsigned int level, int direction) const
	{
		for (int i = 0 ; i < (int)level ; ++i) {
			std::cout << "│";
		}

		if (direction == 0) {
			std::cout << "┌";
		} else if (direction < 0) {
			std::cout << "├";
		} else {
			std::cout << "└";
		}

		if (node == nullptr) {
			std::cout << "─ (empty)\n";
		} else {
			std::cout << "┬";

			std::cout << " " << nng.get_name(node);
			std::cout << "\n";

			this->print_node(Tree::get_left_child(node), level + 1, -1);
			this->print_node(Tree::get_right_child(node), level + 1, 1);
		}
	}

	Tree * tree;
	NodeNameGetter nng;
};

#endif //YGG_DEBUG_HPP
