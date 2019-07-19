#ifndef YGG_BST_HPP
#define YGG_BST_HPP

#include "options.hpp"
#include "size_holder.hpp"
#include "tree_iterator.hpp"
#include "util.hpp"

#include <cassert>
#include <cstddef>
#include <set>
#include <type_traits>

#ifdef YGG_STORE_SEQUENCE
#include "benchmark_sequence.hpp"
#endif

// Only for debugging purposes
#include <fstream>
#include <vector>

namespace ygg {
namespace bst {

#ifdef YGG_STORE_SEQUENCE
class DummySequenceInterface {
public:
	using KeyT = bool; // Set this to whatever your nodes use as key. Anything
	                   // non-numeric will probably blow up
	static KeyT
	    get_key(/* Your node class */) = delete; // You must implement this!
};
#endif

template <class Node>
class DefaultParentContainer {
public:
	Node * get_parent() const noexcept;
	Node *& get_parent() noexcept;
	void set_parent(Node * parent) noexcept;
	static constexpr bool parent_reference = true;

private:
	Node * _bst_parent;
};

// TODO document
template <class Node>
class DefaultFindCallbacks {
public:
	void
	init_root(Node * root) const noexcept
	{
		(void)root;
	};
	void
	descend_left(Node * child) const noexcept
	{
		(void)child;
	};
	void
	descend_right(Node * child) const noexcept
	{
		(void)child;
	};
	void
	found(Node * node) const noexcept
	{
		(void)node;
	};
	void not_found() const noexcept {};

	static DefaultFindCallbacks<Node> dummy;
};

template <class Node, class Tag = int,
          class ParentContainer = DefaultParentContainer<Node>>
class BSTNodeBase {

private:
	/* Determine whether our parent storage allows us to obtain a
	 * reference to the parent pointer. Used for set-by-arithmetic magic. */
	constexpr static auto
	compute_parent_pointer_type()
	{
		if constexpr (ParentContainer::parent_reference) {
			return utilities::TypeHolder<Node *&>{};
		} else {
			return utilities::TypeHolder<Node *>{};
		}
	}

protected:
	Node * _bst_children[2];
	ParentContainer _bst_parent;

	template <class InnerNode>
	friend InnerNode * utilities::go_right_if(bool cond, InnerNode * parent);
	template <class InnerNode>
	friend InnerNode * utilities::go_left_if(bool cond, InnerNode * parent);

public:
	void set_parent(Node * new_parent) noexcept;
	Node * get_parent() const noexcept;
	template <class InnerPC = ParentContainer>
	std::enable_if_t<InnerPC::parent_reference, Node *&> get_parent() const
	    noexcept;

	void set_left(Node * new_left) noexcept;
	void set_right(Node * new_right) noexcept;
	Node *& get_left() noexcept;
	Node *& get_right() noexcept;
	Node * const & get_left() const noexcept;
	Node * const & get_right() const noexcept;

	// Debugging methods TODO remove this
	size_t get_depth() const noexcept;
};

template <class Node, class Options, class Tag = int,
          class Compare = ygg::utilities::flexible_less,
          class ParentContainer = DefaultParentContainer<Node>>
class BinarySearchTree {
public:
	using MyClass =
	    BinarySearchTree<Node, Options, Tag, Compare, ParentContainer>;
	// Node Base
	using NB = BSTNodeBase<Node, Tag, ParentContainer>;
	static_assert(std::is_base_of<NB, Node>::value,
	              "Node class not properly derived from BSTNodeBase");

	/**
	 * @brief Create a new empty red-black tree.
	 */
	BinarySearchTree() noexcept;

	/**
	 * @brief Create a new red-black tree from a different red-black tree.
	 *
	 * The other red-black tree is moved into this one, i.e., using it
	 * afterwards is undefined behavior.
	 *
	 * @param other  The red-black tree that this one is constructed from
	 */
	BinarySearchTree(MyClass && other) noexcept;

	/**
	 * @brief Move-assign an other red-black tree to this one
	 *
	 * The other red-black tree is moved into this one, i.e., using it
	 * afterwards is undefined behavior.
	 *
	 * @param other  The red-black tree that this one is constructed from
	 */
	MyClass & operator=(MyClass && other) noexcept;

	/******************************************************
	 ******************************************************
	 *          Begin of iterator declaration
	 ******************************************************
	 ******************************************************/
private:
	// Class to tell the abstract search tree iterator how to handle our nodes
	class NodeInterface {
	public:
		static Node *
		get_parent(Node * n) noexcept
		{
			return n->NB::get_parent();
		}
		static Node *
		get_left(Node * n) noexcept
		{
			return n->NB::get_left();
		}
		static Node *
		get_right(Node * n) noexcept
		{
			return n->NB::get_right();
		}

		static const Node *
		get_parent(const Node * n) noexcept
		{
			return n->NB::get_parent();
		}
		static const Node *
		get_left(const Node * n) noexcept
		{
			return n->NB::get_left();
		}
		static const Node *
		get_right(const Node * n) noexcept
		{
			return n->NB::get_right();
		}
	};

public:
	// forward, for friendship
	template <bool reverse>
	class const_iterator;

	template <bool reverse>
	class iterator : public internal::IteratorBase<iterator<reverse>, Node,
	                                               NodeInterface, reverse> {
	public:
		using internal::IteratorBase<iterator<reverse>, Node, NodeInterface,
		                             reverse>::IteratorBase;
		iterator(const iterator<reverse> & orig) noexcept
		    : internal::IteratorBase<iterator<reverse>, Node, NodeInterface,
		                             reverse>(orig.n){};
		iterator() noexcept
		    : internal::IteratorBase<iterator<reverse>, Node, NodeInterface,
		                             reverse>(){};

		iterator<reverse> &
		operator=(const iterator<reverse> & orig) noexcept = default;

	private:
		friend class const_iterator<reverse>;
	};

	template <bool reverse>
	class const_iterator
	    : public internal::IteratorBase<const_iterator<reverse>, const Node,
	                                    NodeInterface, reverse> {
	public:
		using internal::IteratorBase<const_iterator<reverse>, const Node,
		                             NodeInterface, reverse>::IteratorBase;
		const_iterator(const const_iterator<reverse> & orig) noexcept
		    : internal::IteratorBase<const_iterator<reverse>, const Node,
		                             NodeInterface, reverse>(orig.n){};
		const_iterator(const iterator<reverse> & orig) noexcept
		    : internal::IteratorBase<const_iterator<reverse>, const Node,
		                             NodeInterface, reverse>(orig.n){};
		const_iterator() noexcept
		    : internal::IteratorBase<const_iterator<reverse>, const Node,
		                             NodeInterface, reverse>(){};

		const_iterator<reverse> &
		operator=(const const_iterator<reverse> & orig) noexcept = default;
	};

	/******************************************************
	 ******************************************************
	 *          End of iterator declaration
	 ******************************************************
	 ******************************************************/

	/**
	 * @brief Finds an element in the tree
	 *
	 * Returns an iterator to the first element that compares equally to <query>.
	 * Note that <query> does not have to be a Node, but can be anything that can
	 * be compared to a Node, i.e., for which
	 *    Compare()(const Node &, const Comparable &)
	 * and
	 *    Compare()(const Comparable &, const Node &)
	 * are defined and implemented. In the case of using the default
	 * ygg::utilities::flexible_less as Compare, that means you have to implement
	 * operator<() for both types.
	 *
	 * @warning Not available for explicitly ordered trees
	 *
	 * @param query An object comparing equally to the element that should be
	 * found.
	 * @returns An iterator to the first element comparing equally to <query>, or
	 * end() if no such element exists
	 */
	template <class Comparable>
	const_iterator<false> find(const Comparable & query) const
#ifndef YGG_STORE_SEQUENCE
	    // Sequence storage involves memory allocation and thus is not noexcept
	    noexcept(noexcept(this->cmp(*this->root, query)))
#endif
	        ;
	template <class Comparable>
	iterator<false> find(const Comparable & query)
#ifndef YGG_STORE_SEQUENCE
	    // Sequence storage involves memory allocation and thus is not noexcept
	    noexcept(noexcept(this->cmp(*this->root, query)))
#endif
	        ;

	// TODO document
	// TODO test
	// TODO noexcept-expression?
	template <class Comparable, class Callbacks = DefaultFindCallbacks<Node>>
	iterator<false> find(const Comparable & query, Callbacks * cbs);

	/**
	 * @brief Upper-bounds an element
	 *
	 * Returns an iterator to the smallest element to which <query> compares as
	 * "less", i.e. the smallest element that is considered go strictly after
	 * <query>.
	 *
	 * Note that <query> does not have to be a Node, but can be anything that can
	 * be compared to a Node, i.e., for which
	 *    Compare()(const Node &, const Comparable &)
	 * and
	 *    Compare()(const Comparable &, const Node &)
	 * are defined and implemented. In the case of using the default
	 * ygg::utilities::flexible_less as Compare, that means you have to implement
	 * operator<() for both types.
	 *
	 * @warning Not available for explicitly ordered trees
	 *
	 * @param query An object comparable to Node that should be upper-bounded
	 * @returns An iterator to the first element comparing "greater" to <query>,
	 * or end() if no such element exists
	 */
	template <class Comparable>
	const_iterator<false> upper_bound(const Comparable & query) const
#ifndef YGG_STORE_SEQUENCE
	    // Sequence storage involves memory allocation and thus is not noexcept
	    noexcept(noexcept(this->cmp(*this->root, query)))
#endif
	        ;

	template <class Comparable>
	iterator<false> upper_bound(const Comparable & query)
#ifndef YGG_STORE_SEQUENCE
	    // Sequence storage involves memory allocation and thus is not noexcept
	    noexcept(noexcept(this->cmp(*this->root, query)))
#endif
	        ;

	/**
	 * @brief Lower-bounds an element
	 *
	 * Returns an iterator to the first element that is not less that <query>,
	 * i.e., that does not have to go before <query>.
	 *
	 * Note that <query> does not have to be a Node, but can be anything that can
	 * be compared to a Node, i.e., for which
	 *    Compare()(const Node &, const Comparable &)
	 * and
	 *    Compare()(const Comparable &, const Node &)
	 * are defined and implemented. In the case of using the default
	 * ygg::utilities::flexible_less as Compare, that means you have to implement
	 * operator<() for both types.
	 *
	 * @warning Not available for explicitly ordered trees
	 *
	 * @param query An object comparable to Node that should be lower-bounded
	 * @returns An iterator to the first element comparing greater-or-equally to
	 * <query>, or end() if no such element exists
	 */
	template <class Comparable>
	const_iterator<false> lower_bound(const Comparable & query) const
#ifndef YGG_STORE_SEQUENCE
	    // Sequence storage involves memory allocation and thus is not noexcept
	    noexcept(noexcept(this->cmp(*this->root, query)))
#endif
	        ;

	template <class Comparable>
	iterator<false> lower_bound(const Comparable & query)
#ifndef YGG_STORE_SEQUENCE
	    // Sequence storage involves memory allocation and thus is not noexcept
	    noexcept(noexcept(this->cmp(*this->root, query)))
#endif
	        ;

	/**
	 * @brief Debugging Method: Draw the Tree as a .dot file
	 *
	 * Outputs the current tree as a .dot file which can be drawn using
	 * graphviz.
	 *
	 * @param filename  The file path where to write the .dot file.
	 */
	template <class NodeTraits>
	void dump_to_dot(const std::string & filename) const;

	// Iteration
	/**
	 * Returns an iterator pointing to the smallest element in the tree.
	 */
	const_iterator<false> cbegin() const noexcept;
	/**
	 * Returns an iterator pointing after the largest element in the tree.
	 */
	const_iterator<false> cend() const noexcept;
	/**
	 * Returns an iterator pointing to the smallest element in the tree.
	 */
	const_iterator<false> begin() const noexcept;
	iterator<false> begin() noexcept;

	/**
	 * Returns an iterator pointing after the largest element in the tree.
	 */
	const_iterator<false> end() const noexcept;
	iterator<false> end() noexcept;

	/**
	 * Returns an reverse iterator pointing to the largest element in the tree.
	 */
	const_iterator<true> crbegin() const noexcept;
	/**
	 * Returns an reverse iterator pointing before the smallest element in the
	 * tree.
	 */
	const_iterator<true> crend() const noexcept;
	/**
	 * Returns an reverse iterator pointing to the largest element in the tree.
	 */
	const_iterator<true> rbegin() const noexcept;
	iterator<true> rbegin() noexcept;

	/**
	 * Returns an reverse iterator pointing before the smallest element in the
	 * tree.
	 */
	const_iterator<true> rend() const noexcept;
	iterator<true> rend() noexcept;

	/**
	 * Returns an iterator pointing to the entry held in node.
	 *
	 * @param node  The node the iterator should point to.
	 */
	const_iterator<false> iterator_to(const Node & node) const noexcept;
	iterator<false> iterator_to(Node & node) noexcept;

	/**
	 * Return the number of elements in the tree.
	 *
	 * This method runs in O(1).
	 *
	 * @warning This method is only available if CONSTANT_TIME_SIZE is set as
	 * option!
	 *
	 * @return The number of elements in the tree.
	 */
	size_t size() const noexcept;

	/**
	 * @brief Removes all elements from the tree.
	 *
	 * Removes all elements from the tree.
	 */
	void clear() noexcept;

	/**
	 * @brief Returns whether the tree is empty
	 *
	 * This method runs in O(1).
	 *
	 * @return true if the tree is empty, false otherwise
	 */
	bool empty() const noexcept;

	// TODO document
	// TODO do we need them anymore?
	Node * get_root() const noexcept;
	static Node * get_parent(Node * n) noexcept;
	static Node * get_left_child(Node * n) noexcept;
	static Node * get_right_child(Node * n) noexcept;

	// Mainly debugging methods
	/// @cond INTERNAL
	bool verify_integrity() const;
	void dbg_verify() const;
	/// @endcond

protected:
	Node * root;

	Node * get_smallest() const noexcept;
	Node * get_largest() const noexcept;
	Node * get_uncle(Node * node) const noexcept;

	Compare cmp;

	SizeHolder<Options::constant_time_size> s;

	/* What follows are debugging tools */
	void dbg_print_tree() const;

	template <class NodeNameGetter>
	void dump_to_dot_base(const std::string & filename,
	                      NodeNameGetter name_getter) const;

	template <class NodeNameGetter>
	void output_node_base(const Node * node, std::ofstream & out,
	                      NodeNameGetter name_getter) const;

	// @cond INTERNAL
	void verify_tree() const;
	void verify_order() const;
	void verify_size() const;
	// @endcond

#ifdef YGG_STORE_SEQUENCE
	typename ygg::utilities::template BenchmarkSequenceStorage<
	    typename Options::SequenceInterface::KeyT>
	    bss;
#endif
};

} // namespace bst
} // namespace ygg

#ifndef YGG_BST_CPP
#include "bst.cpp"
#endif

#endif // YGG_BST_HPP
