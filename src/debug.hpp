//
// Created by lukas on 20.11.17.
//

#ifndef YGG_DEBUG_HPP
#define YGG_DEBUG_HPP

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace ygg {
namespace debug {

class VerifyException : public std::runtime_error {
public:
	VerifyException() : std::runtime_error(""){};
};

void
yggassert(bool condition)
{
	if (!condition) {
		throw VerifyException();
	}
}

template <class Node>
Node *
dbg_find_root(Node * n)
{
	while (n->get_parent() != nullptr) {
		n = n->get_parent();
	}

	return n;
}

template <class Node, class NodeNameGetter>
class TreePrinter {
public:
	TreePrinter(Node * root_in, NodeNameGetter nng_in)
	    : root(root_in), nng(nng_in)
	{}

	void
	reset_root(Node * new_root)
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
	print_node(Node * node, std::vector<std::string> & prefix,
	           int direction) const
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
			if (direction != 0) {
				std::cout << "┬";
			}

			std::cout << " " << nng.get_name(node);
			std::cout << "\n";

			this->print_node(node->get_left(), prefix, -1);
			this->print_node(node->get_right(), prefix, 1);
		}
		if (direction != 0) {
			prefix.pop_back();
		}
	}

	Node * root;
	NodeNameGetter nng;
};

template <class Node, class NodeNameGetter, class EdgeNameGetter>
class TreeDotExport {
public:
	TreeDotExport(Node * root_in, NodeNameGetter nng_in, EdgeNameGetter eng_in)
	    : root(root_in), nng(nng_in), eng(eng_in), buf()
	{}

	void
	reset_root(Node * new_root)
	{
		this->root = new_root;
	}

	std::stringstream &
	get()
	{
		// Reset any previous buffer
		this->buf = std::stringstream();
		this->node_names.clear();

		this->buf << "digraph g {\n";
		this->buf << "  ordering = out;\n\n";
		this->handle_node(this->root);
		this->buf << "}\n";

		return this->buf;
	}

private:
	unsigned int
	get_node_id(Node * node)
	{
		if (this->node_names.find(node) == this->node_names.end()) {
			unsigned int nid = this->node_names.size();
			this->node_names.insert({node, nid});

			this->buf << "\n  " << nid << " [label=\"" << this->nng.get_name(node)
			          << "\"];\n";
			if (node->get_left() == nullptr) {
				this->buf << "  leftdummy" << nid << " [];\n";
			}
			if (node->get_right() == nullptr) {
				this->buf << "  rightdummy" << nid << " [];\n";
			}
			this->buf << "\n";
		}

		return this->node_names[node];
	}
	void
	handle_node(Node * node)
	{
		unsigned int nid = this->get_node_id(node);

		if (node->get_left() != nullptr) {
			unsigned int left_child_id = this->get_node_id(node->get_left());
			this->buf << "  " << nid << " -> " << left_child_id << " [label=\""
			          << this->eng.get_name(node, true) << "\"];\n";
			this->handle_node(node->get_left());
		} else {
			this->buf << "  " << nid << " -> leftdummy" << nid << " [label=\""
			          << this->eng.get_name(node, true) << "\"];\n";
		}

		if (node->get_right() != nullptr) {
			unsigned int right_child_id = this->get_node_id(node->get_right());
			this->buf << "  " << nid << " -> " << right_child_id << " [label=\""
			          << this->eng.get_name(node, false) << "\"];\n";
			this->handle_node(node->get_right());
		} else {
			this->buf << "  " << nid << " -> rightdummy" << nid << " [label=\""
			          << this->eng.get_name(node, false) << "\"];\n";
		}
	}

	Node * root;
	NodeNameGetter nng;
	EdgeNameGetter eng;
	std::stringstream buf;
	std::unordered_map<Node *, unsigned int> node_names;
};

} // namespace debug
} // namespace ygg

#endif // YGG_DEBUG_HPP
