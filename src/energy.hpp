#ifndef YGG_ENERGY_HPP
#define YGG_ENERGY_HPP

#include <vector>

#include "options.hpp"
#include "size_holder.hpp"
#include "tree_iterator.hpp"

namespace ygg {

template <class Node, class Options = DefaultOptions, class Tag = int>
class EnergyTreeNodeBase {
public:
	Node * _et_left;
	Node * _et_right;
	Node * _et_parent;

	size_t _et_size;
	size_t _et_energy;

	// Debugging
	size_t get_depth() const noexcept;
};

template <class Node, class Options = DefaultOptions, class Tag = int,
          class Compare = ygg::utilities::flexible_less>
class EnergyTree {
	using MyClass = EnergyTree<Node, Options, Tag, Compare>;
	using NB = EnergyTreeNodeBase<Node, Options, Tag>;

	static_assert(std::is_base_of<NB, Node>::value,
	              "Node class not properly derived from EnergyTreeNodeBase");

public:
	EnergyTree();
	EnergyTree(MyClass && other);
	MyClass & operator=(MyClass && other);

private:
	/******************************************************
	 ******************************************************
	 *          Begin of iterator declaration
	 ******************************************************
	 ******************************************************/

	// Class to tell the abstract search tree iterator how to handle our nodes
	class NodeInterface {
	public:
		static Node *
		get_parent(Node * n)
		{
			return n->NB::_et_parent;
		}
		static Node *
		get_left(Node * n)
		{
			return n->NB::_et_left;
		}
		static Node *
		get_right(Node * n)
		{
			return n->NB::_et_right;
		}

		static const Node *
		get_parent(const Node * n)
		{
			return n->NB::_et_parent;
		}
		static const Node *
		get_left(const Node * n)
		{
			return n->NB::_et_left;
		}
		static const Node *
		get_right(const Node * n)
		{
			return n->NB::_et_right;
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
		iterator(const iterator<reverse> & orig)
		    : internal::IteratorBase<iterator<reverse>, Node, NodeInterface,
		                             reverse>(orig.n){};
		iterator()
		    : internal::IteratorBase<iterator<reverse>, Node, NodeInterface,
		                             reverse>(){};

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
		const_iterator(const const_iterator<reverse> & orig)
		    : internal::IteratorBase<const_iterator<reverse>, const Node,
		                             NodeInterface, reverse>(orig.n){};
		const_iterator(const iterator<reverse> & orig)
		    : internal::IteratorBase<const_iterator<reverse>, const Node,
		                             NodeInterface, reverse>(orig.n){};
		const_iterator()
		    : internal::IteratorBase<const_iterator<reverse>, const Node,
		                             NodeInterface, reverse>(){};
	};
	/******************************************************
	 ******************************************************
	 *          End of iterator declaration
	 ******************************************************
	 ******************************************************/

	/**
	 * @brief Inserts <node> into the tree
	 *
	 * Inserts <node> into the tree.
	 *
	 * *Warning*: Please note that after calling insert() on a node (and before
	 * removing that node again), that node *may not move in memory*. A common
	 * pitfall is to store nodes in a std::vector (or other STL container), which
	 * reallocates (and thereby moves objects around).
	 *
	 * @warning Not available for explicitly ordered trees
	 *
	 * @param   Node  The node to be inserted.
	 */
	void insert(Node & node);
	void insert(Node & node, Node & hint);
	void insert(Node & node, iterator<false> hint);

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
	const_iterator<false> find(const Comparable & query) const;
	template <class Comparable>
	iterator<false> find(const Comparable & query);

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
	const_iterator<false> upper_bound(const Comparable & query) const;
	template <class Comparable>
	iterator<false> upper_bound(const Comparable & query);

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
	const_iterator<false> lower_bound(const Comparable & query) const;
	template <class Comparable>
	iterator<false> lower_bound(const Comparable & query);

	
	// Iteration
	/**
	 * Returns an iterator pointing to the smallest element in the tree.
	 */
	const_iterator<false> cbegin() const;
	/**
	 * Returns an iterator pointing after the largest element in the tree.
	 */
	const_iterator<false> cend() const;
	/**
	 * Returns an iterator pointing to the smallest element in the tree.
	 */
	const_iterator<false> begin() const;
	iterator<false> begin();

	/**
	 * Returns an iterator pointing after the largest element in the tree.
	 */
	const_iterator<false> end() const;
	iterator<false> end();

	/**
	 * Returns an reverse iterator pointing to the largest element in the tree.
	 */
	const_iterator<true> crbegin() const;
	/**
	 * Returns an reverse iterator pointing before the smallest element in the
	 * tree.
	 */
	const_iterator<true> crend() const;
	/**
	 * Returns an reverse iterator pointing to the largest element in the tree.
	 */
	const_iterator<true> rbegin() const;
	iterator<true> rbegin();

	/**
	 * Returns an reverse iterator pointing before the smallest element in the
	 * tree.
	 */
	const_iterator<true> rend() const;
	iterator<true> rend();

	/**
	 * Returns an iterator pointing to the entry held in node.
	 *
	 * @param node  The node the iterator should point to.
	 */
	const_iterator<false> iterator_to(const Node & node) const;
	iterator<false> iterator_to(Node & node);

	/**
	 * @brief Removes <node> from the tree
	 *
	 * Removes <node> from the tree.
	 *
	 * @param   Node  The node to be removed.
	 */
	void remove(Node & node);

	/**
	 * @brief Removes all elements from the tree.
	 *
	 * Removes all elements from the tree.
	 */
	void clear();

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
	size_t size() const;
	
	/**
	 * @brief Returns whether the tree is empty
	 *
	 * This method runs in O(1).
	 *
	 * @return true if the tree is empty, false otherwise
	 */
	bool empty() const;

	void dbg_verify() const;
	bool verify_integrity() const;

private:
	void rebuild_below(Node * node);
	Node * get_smallest() const;
	Node * get_largest() const;

	Node * root;
	Compare cmp;
	SizeHolder<Options::constant_time_size> s;

	std::vector<Node *> rebuild_buffer;

	void dbg_verify_sizes() const;
	void dbg_verify_energy() const;
	void dbg_verify_tree(Node * node = nullptr) const;
	void dbg_verify_size() const;
};

} // namespace ygg

#include "energy.cpp"

#endif // YGG_ENERGY_HPP
