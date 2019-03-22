#ifndef YGG_BST_CPP
#define YGG_BST_CPP

#include "bst.hpp"

namespace ygg {

namespace bst {

template <class Node>
Node *
DefaultParentContainer<Node>::get_parent() const
{
	return this->_bst_parent;
}

template <class Node>
void
DefaultParentContainer<Node>::set_parent(Node * parent)
{
	this->_bst_parent = parent;
}

template <class Node, class Tag, class ParentContainer>
size_t
BSTNodeBase<Node, Tag, ParentContainer>::get_depth() const noexcept
{
	size_t depth = 0;
	const Node * n = (const Node *)this;

	while (n->get_parent() != nullptr) {
		depth++;
		n = n->get_parent();
	}

	return depth;
}

template <class Node, class Tag, class ParentContainer>
Node *
BSTNodeBase<Node, Tag, ParentContainer>::get_parent() const
{
	return this->_bst_parent.get_parent();
	;
}

template <class Node, class Tag, class ParentContainer>
Node *&
BSTNodeBase<Node, Tag, ParentContainer>::get_left()
{
	return this->_bst_left;
}

template <class Node, class Tag, class ParentContainer>
Node *&
BSTNodeBase<Node, Tag, ParentContainer>::get_right()
{
	return this->_bst_right;
}

template <class Node, class Tag, class ParentContainer>
Node * const &
BSTNodeBase<Node, Tag, ParentContainer>::get_left() const
{
	return this->_bst_left;
}

template <class Node, class Tag, class ParentContainer>
Node * const &
BSTNodeBase<Node, Tag, ParentContainer>::get_right() const
{
	return this->_bst_right;
}

template <class Node, class Tag, class ParentContainer>
void
BSTNodeBase<Node, Tag, ParentContainer>::set_parent(Node * new_parent)
{
	this->_bst_parent.set_parent(new_parent);
}

template <class Node, class Tag, class ParentContainer>
void
BSTNodeBase<Node, Tag, ParentContainer>::set_left(Node * new_left)
{
	this->_bst_left = new_left;
}

template <class Node, class Tag, class ParentContainer>
void
BSTNodeBase<Node, Tag, ParentContainer>::set_right(Node * new_right)
{
	this->_bst_right = new_right;
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
BinarySearchTree<Node, Options, Tag, Compare,
                 ParentContainer>::BinarySearchTree()
    : root(nullptr)
{}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
BinarySearchTree<Node, Options, Tag, Compare,
                 ParentContainer>::BinarySearchTree(MyClass && other)
{
	this->root = other.root;
	other.root = nullptr;
	this->s = other.s;
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer> &
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::operator=(
    MyClass && other)
{
	this->root = other.root;
	other.root = nullptr;
	this->s = other.s;
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer> &
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::mimic(
    const MyClass & other)
{
	auto from_it = other.begin();
	auto dest_it = this->begin();

	while (from_it != other.end()) {
		assert(dest_it != this->end());

		*dest_it = *from_it;

		++dest_it;
		++from_it;
	}

	this->s = other.s;

	return *this;
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
void
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::clear()
{
	this->root = nullptr;
	this->s.set(0);
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
Node *
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::get_uncle(
    Node * node) const
{
	Node * parent = node->NB::get_parent();
	Node * grandparent = parent->NB::get_parent();

	if (grandparent->NB::get_left() == parent) {
		return grandparent->NB::get_right();
	} else {
		return grandparent->NB::get_left();
	}
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
bool
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::verify_order()
    const
{
	for (const Node & n : *this) {
		if (n.NB::get_left() != nullptr) {
			// left may not be larger
			if (this->cmp(n, *(n.NB::get_left()))) {
				assert(false);
				return false;
			}
		}

		if (n.NB::get_right() != nullptr) {
			// right may not be smaller
			if (this->cmp(*(n.NB::get_right()), n)) {
				assert(false);
				return false;
			}
		}
	}

	return true;
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
bool
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::verify_tree()
    const
{
	if (this->root == nullptr) {
		return true;
	}

	Node * cur = this->root;
	while (cur->NB::get_left() != nullptr) {
		cur = cur->NB::get_left();
		if (cur->NB::get_left() == cur) {
			assert(false);
			return (false);
		}
	}

	std::set<Node *> seen;

	while (cur != nullptr) {
		if (seen.find(cur) != seen.end()) {
			assert(false);
			return false;
		}
		seen.insert(cur);

		if (cur->NB::get_left() != nullptr) {
			if (cur->NB::get_left()->NB::get_parent() != cur) {
				assert(false);
				return false;
			}
			if (cur->NB::get_right() == cur) {
				assert(false);
				return false;
			}
		}

		if (cur->NB::get_right() != nullptr) {
			if (cur->NB::get_right()->NB::get_parent() != cur) {
				assert(false);
				return false;
			}
			if (cur->NB::get_right() == cur) {
				assert(false);
				return false;
			}
		}

		/*
		 * Begin: find the next-largest vertex
		 */
		if (cur->NB::get_right() != nullptr) {
			// go to smallest larger-or-equal child
			cur = cur->NB::get_right();
			while (cur->NB::get_left() != nullptr) {
				cur = cur->NB::get_left();
			}
		} else {
			// go up

			// skip over the nodes already visited
			// TODO have a 'parents_left_child_is' / 'parents_right_child_is'
			// function?
			while ((cur->NB::get_parent() != nullptr) &&
			       (cur->NB::get_parent()->NB::get_right() ==
			        cur)) { // these are the nodes which are smaller and were already
				              // visited
				cur = cur->NB::get_parent();
			}

			// go one further up
			if (cur->NB::get_parent() == nullptr) {
				// done
				cur = nullptr;
			} else {
				// go up
				cur = cur->NB::get_parent();
			}
		}
		/*
		 * End: find the next-largest vertex
		 */
	}

	return true;
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
bool
BinarySearchTree<Node, Options, Tag, Compare,
                 ParentContainer>::verify_integrity() const
{
	unsigned int dummy;

	bool tree_okay = this->verify_tree();
	bool order_okay = this->verify_order();

	// TODO move to new debug scheme
	debug::yggassert(tree_okay);
	debug::yggassert(order_okay);

	assert(tree_okay && order_okay);

	(void)dummy;

	return tree_okay && order_okay;
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
template <class NodeNameGetter>
void
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::
    dump_to_dot_base(const std::string & filename,
                     NodeNameGetter name_getter) const
{
	std::ofstream dotfile;
	dotfile.open(filename);
	dotfile << "digraph G {\n";
	this->output_node_base(this->root, dotfile, name_getter);
	dotfile << "}\n";
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
template <class NodeTraits>
void
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::dump_to_dot(
    const std::string & filename) const
{
	this->dump_to_dot_base(
	    filename, [&](const Node * node) { return NodeTraits::get_id(node); });
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
template <class NodeNameGetter>
void
BinarySearchTree<Node, Options, Tag, Compare,
                 ParentContainer>::output_node_base(const Node * node,
                                                    std::ofstream & out,
                                                    NodeNameGetter name_getter)
    const
{
	if (node == nullptr) {
		return;
	}

	out << "  " << std::to_string((long unsigned int)node) << "[ "
	    << " label=\"" << name_getter(node) << "\"]\n";

	if (node->NB::get_parent() != nullptr) {
		std::string label;
		if (node->NB::get_parent()->NB::get_left() == node) {
			label = std::string("L");
		} else {
			label = std::string("R");
		}

		out << "  " << std::to_string((long unsigned int)node->NB::get_parent())
		    << " -> " << std::to_string((long unsigned int)node) << "[ label=\""
		    << label << "\"]\n";
	}

	this->output_node_base(node->NB::get_left(), out, name_getter);
	this->output_node_base(node->NB::get_right(), out, name_getter);
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
size_t
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::size() const
{
	return this->s.get();
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
bool
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::empty() const
{
	return this->root == nullptr;
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
Node *
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::get_smallest()
    const
{
	Node * smallest = this->root;
	if (smallest == nullptr) {
		return nullptr;
	}

	while (smallest->NB::get_left() != nullptr) {
		smallest = smallest->NB::get_left();
	}

	return smallest;
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
Node *
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::get_largest()
    const
{
	Node * largest = this->root;
	if (largest == nullptr) {
		return nullptr;
	}

	while (largest->NB::get_right() != nullptr) {
		largest = largest->NB::get_right();
	}

	return largest;
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
typename BinarySearchTree<Node, Options, Tag, Compare,
                          ParentContainer>::template const_iterator<false>
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::iterator_to(
    const Node & node) const
{
	return const_iterator<false>(&node);
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
typename BinarySearchTree<Node, Options, Tag, Compare,
                          ParentContainer>::template iterator<false>
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::iterator_to(
    Node & node)
{
	return iterator<false>(&node);
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
typename BinarySearchTree<Node, Options, Tag, Compare,
                          ParentContainer>::template const_iterator<false>
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::cbegin() const
{
	Node * smallest = this->get_smallest();
	if (smallest == nullptr) { // TODO what the hell?
		return const_iterator<false>(nullptr);
	}

	return const_iterator<false>(smallest);
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
typename BinarySearchTree<Node, Options, Tag, Compare,
                          ParentContainer>::template const_iterator<false>
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::cend() const
{
	return const_iterator<false>(nullptr);
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
typename BinarySearchTree<Node, Options, Tag, Compare,
                          ParentContainer>::template const_iterator<false>
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::begin() const
{
	return this->cbegin();
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
typename BinarySearchTree<Node, Options, Tag, Compare,
                          ParentContainer>::template iterator<false>
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::begin()
{
	Node * smallest = this->get_smallest();
	if (smallest == nullptr) {
		return iterator<false>(nullptr);
	}

	return iterator<false>(smallest);
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
typename BinarySearchTree<Node, Options, Tag, Compare,
                          ParentContainer>::template const_iterator<false>
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::end() const
{
	return this->cend();
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
typename BinarySearchTree<Node, Options, Tag, Compare,
                          ParentContainer>::template iterator<false>
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::end()
{
	return iterator<false>(nullptr);
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
typename BinarySearchTree<Node, Options, Tag, Compare,
                          ParentContainer>::template const_iterator<true>
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::crbegin() const
{
	Node * largest = this->get_largest();
	if (largest == nullptr) {
		return const_iterator<true>(nullptr);
	}

	return const_iterator<true>(largest);
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
typename BinarySearchTree<Node, Options, Tag, Compare,
                          ParentContainer>::template const_iterator<true>
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::crend() const
{
	return const_iterator<true>(nullptr);
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
typename BinarySearchTree<Node, Options, Tag, Compare,
                          ParentContainer>::template const_iterator<true>
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::rbegin() const
{
	return this->crbegin();
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
typename BinarySearchTree<Node, Options, Tag, Compare,
                          ParentContainer>::template const_iterator<true>
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::rend() const
{
	return this->crend();
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
typename BinarySearchTree<Node, Options, Tag, Compare,
                          ParentContainer>::template iterator<true>
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::rbegin()
{
	Node * largest = this->get_largest();
	if (largest == nullptr) {
		return iterator<true>(nullptr);
	}

	return iterator<true>(largest);
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
typename BinarySearchTree<Node, Options, Tag, Compare,
                          ParentContainer>::template iterator<true>
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::rend()
{
	return iterator<true>(nullptr);
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
template <class Comparable, class Callbacks>
typename BinarySearchTree<Node, Options, Tag, Compare,
                          ParentContainer>::template iterator<false>
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::find(
    const Comparable & query, Callbacks * cbs)
{
	Node * cur = this->root;
	cbs->init_root(cur);

	while (cur != nullptr) {
		if (this->cmp(*cur, query)) {
			cur = cur->NB::get_right();
			cbs->descend_right(cur);
		} else if (this->cmp(query, *cur)) {
			cur = cur->NB::get_left();
			cbs->descend_left(cur);
		} else {
			cbs->found(cur);
			return iterator<false>(cur);
		}
	}

	cbs->not_found();
	return this->end();
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
template <class Comparable>
typename BinarySearchTree<Node, Options, Tag, Compare,
                          ParentContainer>::template iterator<false>
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::find(
    const Comparable & query)
{
	Node * cur = this->root;
	Node * last_left = nullptr;

	while (cur != nullptr) {
		if (this->cmp(*cur, query)) {
			cur = cur->NB::get_right();
		} else {
			last_left = cur;
			cur = cur->NB::get_left();
		}
	}

	if ((last_left != nullptr) && (!this->cmp(query, *last_left))) {
		return iterator<false>(last_left);
	} else {
		return this->end();
	}
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
template <class Comparable>
typename BinarySearchTree<Node, Options, Tag, Compare,
                          ParentContainer>::template const_iterator<false>
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::find(
    const Comparable & query) const
{
	return const_iterator<false>(const_cast<decltype(this)>(this)->find(query));
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
template <class Comparable>
typename BinarySearchTree<Node, Options, Tag, Compare,
                          ParentContainer>::template iterator<false>
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::lower_bound(
    const Comparable & query)
{
	Node * cur = this->root;
	Node * last_left = nullptr;

	while (cur != nullptr) {
		if (this->cmp(*cur, query)) {
			cur = cur->NB::get_right();
		} else {
			last_left = cur;
			cur = cur->NB::get_left();
		}
	}

	if (last_left != nullptr) {
		return iterator<false>(last_left);
	} else {
		return this->end();
	}
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
template <class Comparable>
typename BinarySearchTree<Node, Options, Tag, Compare,
                          ParentContainer>::template iterator<false>
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::upper_bound(
    const Comparable & query)
{
	Node * cur = this->root;
	Node * last_left = nullptr;

	while (cur != nullptr) {
		if (this->cmp(query, *cur)) {
			last_left = cur;
			cur = cur->get_left();
		} else {
			cur = cur->get_right();
		}
	}

	if (last_left != nullptr) {
		return iterator<false>(last_left);
	} else {
		return this->end();
	}
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
template <class Comparable>
typename BinarySearchTree<Node, Options, Tag, Compare,
                          ParentContainer>::template const_iterator<false>
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::upper_bound(
    const Comparable & query) const
{
	return const_iterator<false>(const_cast<MyClass *>(this)->upper_bound(query));
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
template <class Comparable>
typename BinarySearchTree<Node, Options, Tag, Compare,
                          ParentContainer>::template const_iterator<false>
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::lower_bound(
    const Comparable & query) const
{
	return const_iterator<false>(const_cast<MyClass *>(this)->lower_bound(query));
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
Node *
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::get_root() const
{
	return this->root;
}

} // namespace bst
} // namespace ygg

#endif // YGG_BST_CPP
