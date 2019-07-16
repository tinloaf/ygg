#ifndef YGG_BST_CPP
#define YGG_BST_CPP

#include "bst.hpp"

#include "debug.hpp"

namespace ygg {

namespace bst {

template <class Node>
Node *
DefaultParentContainer<Node>::get_parent() const
{
	return this->_bst_parent;
}

template <class Node>
Node *&
DefaultParentContainer<Node>::get_parent()
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
	const Node * n = static_cast<const Node *>(this);

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
}

template <class Node, class Tag, class ParentContainer>
template <class InnerPC>
std::enable_if_t<InnerPC::parent_reference, Node *&>
BSTNodeBase<Node, Tag, ParentContainer>::get_parent()
{
	return this->_bst_parent.get_parent();
}

template <class Node, class Tag, class ParentContainer>
Node *&
BSTNodeBase<Node, Tag, ParentContainer>::get_left()
{
	return this->_bst_children[0];
}

template <class Node, class Tag, class ParentContainer>
Node *&
BSTNodeBase<Node, Tag, ParentContainer>::get_right()
{
	return this->_bst_children[1];
}

template <class Node, class Tag, class ParentContainer>
Node * const &
BSTNodeBase<Node, Tag, ParentContainer>::get_left() const
{
	return this->_bst_children[0];
}

template <class Node, class Tag, class ParentContainer>
Node * const &
BSTNodeBase<Node, Tag, ParentContainer>::get_right() const
{
	return this->_bst_children[1];
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
	this->_bst_children[0] = new_left;
}

template <class Node, class Tag, class ParentContainer>
void
BSTNodeBase<Node, Tag, ParentContainer>::set_right(Node * new_right)
{
	this->_bst_children[1] = new_right;
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
void
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::verify_order()
    const
{
	for (const Node & n : *this) {
		if (n.NB::get_left() != nullptr) {
			// left may not be larger
			debug::yggassert(!(this->cmp(n, *(n.NB::get_left()))));
		}

		if (n.NB::get_right() != nullptr) {
			// right may not be smaller
			debug::yggassert(!(this->cmp(*(n.NB::get_right()), n)));
		}
	}
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
void
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::verify_tree()
    const
{
	if (this->root == nullptr) {
		return;
	}

	Node * cur = this->root;
	while (cur->NB::get_left() != nullptr) {
		debug::yggassert(cur->NB::get_left() != cur->NB::get_right());
		cur = cur->NB::get_left();
		debug::yggassert(cur->NB::get_left() != cur);
		debug::yggassert(cur->NB::get_right() != cur);
	}

	std::set<Node *> seen;

	while (cur != nullptr) {
		debug::yggassert(cur->NB::get_left() != cur);
		debug::yggassert(cur->NB::get_right() != cur);
		if (cur->NB::get_left() != nullptr) {
			debug::yggassert(cur->NB::get_left() != cur->NB::get_right());
		}

		debug::yggassert(seen.find(cur) == seen.end());
		seen.insert(cur);

		if (cur->NB::get_left() != nullptr) {
			debug::yggassert(cur->NB::get_left()->NB::get_parent() == cur);
			debug::yggassert(cur->NB::get_left()->NB::get_left() != cur);
			debug::yggassert(cur->NB::get_left()->NB::get_right() != cur);
			debug::yggassert(cur->NB::get_left() != cur);
		}

		if (cur->NB::get_right() != nullptr) {
			debug::yggassert(cur->NB::get_right()->NB::get_parent() == cur);
			debug::yggassert(cur->NB::get_right()->NB::get_left() != cur);
			debug::yggassert(cur->NB::get_right()->NB::get_right() != cur);
			debug::yggassert(cur->NB::get_right() != cur);
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
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
void
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::dbg_verify()
    const
{
	this->verify_tree();
	this->verify_order();
	this->verify_size();
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
bool
BinarySearchTree<Node, Options, Tag, Compare,
                 ParentContainer>::verify_integrity() const
{
	try {
		this->dbg_verify();
	} catch (debug::VerifyException & e) {
		return false;
	}

	return true;
}

template <class Node, class Options, class Tag, class Compare,
          class ParentContainer>
void
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::verify_size()
    const
{
	if constexpr (Options::constant_time_size) {
		size_t count = 0;
		for (const Node & n : *this) {
			(void)n;
			count++;
		}

		debug::yggassert(count == this->size());
	}
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

	out << "  " << std::hex << node << std::dec << "[ "
	    << " label=\"" << name_getter(node) << "\"]\n";

	if (node->NB::get_parent() != nullptr) {
		std::string label;
		if (node->NB::get_parent()->NB::get_left() == node) {
			label = std::string("L");
		} else {
			label = std::string("R");
		}

		out << "  " << std::hex << node->NB::get_parent() << std::dec << " -> "
		    << std::hex << node << std::dec << "[ label=\"" << label << "\"]\n";
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
void
BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>::dbg_print_tree()
    const
{
	using NNG = ygg::debug::GenericNodeNameGetter<Node>;
	ygg::debug::TreePrinter<Node, NNG> tp(this->root, NNG());
	tp.print();
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
#ifdef YGG_STORE_SEQUENCE
	// TODO point to the query?
	this->bss.register_search(reinterpret_cast<const void *>(&query),
	                          Options::SequenceInterface::get_key(query));
#endif

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
#ifdef YGG_STORE_SEQUENCE
	this->bss.register_search(reinterpret_cast<const void *>(&query),
	                          Options::SequenceInterface::get_key(query));
#endif

	Node * cur = this->root;
	Node * last_left = nullptr;

	while (cur != nullptr) {
		if constexpr (Options::micro_avoid_conditionals) {
			(void)last_left;

			if (__builtin_expect(
			        (!this->cmp(*cur, query)) && (!this->cmp(query, *cur)), false)) {
				return iterator<false>(cur);
			}
			cur = utilities::go_right_if(this->cmp(*cur, query), cur);
		} else {
			if (this->cmp(*cur, query)) {
				cur = cur->NB::get_right();
			} else {
				last_left = cur;
				cur = cur->NB::get_left();
			}
		}
	}

	if constexpr (!Options::micro_avoid_conditionals) {
		if ((last_left != nullptr) && (!this->cmp(query, *last_left))) {
			return iterator<false>(last_left);
		} else {
			return this->end();
		}
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
#ifdef YGG_STORE_SEQUENCE
	this->bss.register_lbound(reinterpret_cast<const void *>(&query),
	                          Options::SequenceInterface::get_key(query));
#endif

	// TODO avoid conditionals!
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
#ifdef YGG_STORE_SEQUENCE
	this->bss.register_ubound(reinterpret_cast<const void *>(&query),
	                          Options::SequenceInterface::get_key(query));
#endif

	// TODO avoid conditionals!
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
