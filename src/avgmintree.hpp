#ifndef YGG_AVGMINTREE_H
#define YGG_AVGMINTREE_H

#include "options.hpp"
#include "size_holder.hpp"
#include "tree_iterator.hpp"

#include <stddef.h>
#include <cmath>
#include <functional>

namespace ygg {

// Forward
template <class Node, class Options = DefaultOptions, class Tag = int>
class AvgMinTreeNodeBase;
template <class Node, class NodeTraits, class Options, class Tag, class Compare>
class AvgMinTree;

namespace avgmintree_internal {
/// @cond INTERNAL

template <class Tree, bool enable>
struct dbg_verify_size_helper
{
	void operator()(const Tree & t, size_t node_count);
};

template <class Tree>
struct dbg_verify_size_helper<Tree, true>
{
	void
	operator()(const Tree & t, size_t node_count)
	{
		assert(t.size() == node_count);
		// Usage in assertion is not enough for GCC to realize that the variable is
		// being used.
		(void)t;
		(void)node_count;
	}
};

template <class Tree>
struct dbg_verify_size_helper<Tree, false>
{
	void
	operator()(const Tree & t, size_t node_count)
	{
		(void)t;
		(void)node_count;
	}
};
/// @endcond
} // namespace avgmintree_internal

/**
 * @brief Base class (template) to supply your node class with metainformation
 *
 * The class you use as nodes for the Zip Tree *must* derive from this
 * class (template). It supplies your class with the necessary members to
 * contain the linking between the tree nodes.
 *
 * @tparam Node    The node class itself. Yes, that's the class derived from
 * this template. This sounds weird, but is correct. See the examples if you're
 * confused.
 * @tparam options  The options class (a version of TreeOptions) that you
 * parameterize the tree with. (See the options parameter of AvgMinTree.)
 * @tparam Tag 		The tag used to identify the tree that this node should
 * be inserted into. See AvgMinTree for details.
 */
template <class Node, class Options, class Tag>
class AvgMinTreeNodeBase {
public:
	Node * _zt_parent = nullptr;
	Node * _zt_left = nullptr;
	Node * _zt_right = nullptr;

	Node *
	get_parent() const noexcept
	{
		return this->_zt_parent;
	}

	Node *
	get_left() const noexcept
	{
		return this->_zt_left;
	}

	Node *
	get_right() const noexcept
	{
		return this->_zt_right;
	}

	// Debugging methods
	size_t get_depth() const noexcept;

private:
	template <class FNode, class FNodeTraits, class FOptions, class FTag,
	          class FCompare>
	friend class AvgMinTree;

	// TODO make types configurable?
	size_t size;
	size_t length_sum;
};

template <class Node>
class AvgMinTreeDefaultNodeTraits {
public:
	// clang-format off
  /*
   * Callbacks for Zipping
   */
  void init_zipping(Node * to_be_deleted) const noexcept {(void)to_be_deleted;};
  void delete_without_zipping(Node * to_be_deleted) const noexcept {(void)to_be_deleted;};
  void before_zip_from_left(Node * left_head) const noexcept {(void)left_head;};
  void before_zip_from_right(Node * right_head) const noexcept {(void)right_head;};
  void zipping_ended_left_without_tree(Node * prev_left_head) const noexcept {(void)prev_left_head;};
  void zipping_ended_right_without_tree(Node * prev_right_head) const noexcept {(void)prev_right_head;};  
  void before_zip_tree_from_left(Node * left_head) const noexcept {(void)left_head;};
  void before_zip_tree_from_right(Node * right_head) const noexcept {(void)right_head;};
  void zipping_done(Node * head, Node * tail) const noexcept {(void)head; (void)tail;}
  
  /*
   * Callbacks for Unzipping
   */
  void init_unzipping(Node * to_be_inserted) const noexcept {(void) to_be_inserted;};
  void unzip_to_left(Node * n) const noexcept {(void)n;}
  void unzip_to_right(Node * n) const noexcept {(void)n;}
  void unzip_done(Node * unzip_root, Node * left_spine_end, Node * right_spine_end) const noexcept
  {
    (void) unzip_root;
    (void) left_spine_end;
    (void) right_spine_end;
  }
	// clang-format on
};

// TODO NodeTraits

/**
 * @brief The Zip Tree
 *
 * This is the main Zip Tree class.
 *
 * @tparam Node         The node class for this tree. It must be derived from
 * AvgMinTreeNodeBase.
 * @tparam NodeTraits   A class implementing various hooks and functions on your
 * node class. See DOCTODO for details.
 * @tparam Options			The TreeOptions class specifying the
 * parameters of this AvgMinTree. See the TreeOptions and TreeFlags classes for
 * details.
 * @tparam Tag					An class tag that identifies
 * this tree. Can be used to insert the same nodes into multiple trees. See
 * DOCTODO for details. Can be any class, the class can be empty.
 * @tparam Compare      A compare class. The Zip Tree follows STL
 * semantics for 'Compare'. Defaults to ygg::utilities::flexible_less. Implement
 * operator<(const Node & lhs, const Node & rhs) if you want to use it.
 * @tparam RankGetter   A class that must implement a static size_t
 * get_rank(const Node &) function that returns the rank of a node. If you
 * implement this yourself (instead of using the provided default), you are
 * responsible of making sure that the ranks uphold the assumptions that zip
 * trees make regarding the ranks.
 */

template <class Node, class NodeTraits, class Options = DefaultOptions,
          class Tag = int, class Compare = ygg::rbtree_internal::flexible_less>
class AvgMinTree {
public:
	using NB = AvgMinTreeNodeBase<Node, Options, Tag>;
	using my_type = AvgMinTree<Node, NodeTraits, Options, Tag, Compare>;

	/**
	 * @brief Construct a new empty Zip Tree.
	 */
	AvgMinTree() noexcept;

	static_assert(std::is_base_of<NB, Node>::value,
	              "Node class not properly derived from node base!");

private:
	// Class to tell the abstract search tree iterator how to handle
	// our nodes
	class NodeInterface {
	public:
		static Node *
		get_parent(Node * n)
		{
			return n->NB::_zt_parent;
		}
		static Node *
		get_left(Node * n)
		{
			return n->NB::_zt_left;
		}
		static Node *
		get_right(Node * n)
		{
			return n->NB::_zt_right;
		}

		static const Node *
		get_parent(const Node * n)
		{
			return n->NB::_zt_parent;
		}
		static const Node *
		get_left(const Node * n)
		{
			return n->NB::_zt_left;
		}
		static const Node *
		get_right(const Node * n)
		{
			return n->NB::_zt_right;
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

	/**
	 * @brief Inserts <node> into the tree
	 *
	 * Inserts <node> into the tree.
	 *
	 * *Warning*: Please note that after calling insert() on a node (and before
	 * removing that node again), that node *may not move in memory*. A common
	 * pitfall is to store nodes in a std::vector (or other STL container), which
	 * reallocates (and thereby moves objecs around).
	 *
	 * For zip trees, the hinted version is equivalent to the unhinted insertion.
	 *
	 * @param   Node  The node to be inserted.
	 */
	void insert(Node & node) noexcept;
	void insert(Node & node, Node & hint) noexcept;

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
	 * @param query An object comparable to Node that should be lower-bounded
	 * @returns An iterator to the first element comparing greater-or-equally to
	 * <query>, or end() if no such element exists
	 */
	template <class Comparable>
	const_iterator<false> lower_bound(const Comparable & query) const;
	template <class Comparable>
	iterator<false> lower_bound(const Comparable & query);

	/**
	 * @brief Removes <node> from the tree
	 *
	 * Removes <node> from the tree.
	 *
	 * @param   Node  The node to be removed.
	 */
	void remove(Node & node) noexcept;

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
	 * @param query An object comparing equally to the element that should be
	 * found.
	 * @returns An iterator to the first element comparing equally to <query>, or
	 * end() if no such element exists
	 */
	template <class Comparable>
	const_iterator<false> find(const Comparable & query) const;
	template <class Comparable>
	iterator<false> find(const Comparable & query);

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

	// Debugging methods
	void dbg_verify() const;

	/**
	 * @brief Debugging Method: Draw the Tree as a .dot file
	 *
	 * Outputs the current tree as a .dot file which can be drawn using
	 * graphviz.
	 *
	 * @param filename  The file path where to write the .dot file.
	 */
	void dump_to_dot(const std::string & filename) const;

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

	Node * get_root() const;

	/**
	 * @brief Removes all elements from the tree.
	 *
	 * Removes all elements from the tree.
	 */
	void clear();

private:
	Node * root;
	Compare cmp;

	void unzip(Node & oldn, Node & newn) noexcept;
	void zip(Node & old_root) noexcept;

	Node * get_smallest() const;
	Node * get_largest() const;

	// Debugging methods
	void dbg_verify_consistency(Node * sub_root, Node * lower_bound,
	                            Node * upper_bound) const;
	void dbg_verify_size() const;

	template <class NodeNameGetter>
	void dump_to_dot_base(const std::string & filename,
	                      NodeNameGetter name_getter) const;

	template <class NodeNameGetter>
	void output_node_base(const Node * node, std::ofstream & out,
	                      NodeNameGetter name_getter) const;

	SizeHolder<Options::constant_time_size> s;
};

} // namespace ygg

#endif

#include "avgmintree.cpp"
