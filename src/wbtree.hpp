#ifndef YGG_WBTREE_HPP
#define YGG_WBTREE_HPP

#include <cassert>
#include <cstddef>
#include <set>
#include <type_traits>

#include "bst.hpp"
#include "debug.hpp"
#include "options.hpp"
#include "size_holder.hpp"
#include "tree_iterator.hpp"

// Only for debugging purposes
#include <fstream>
#include <vector>

namespace ygg {
/**
 * @brief Base class (template) to supply your node class with metainformation
 *
 * The class you use as nodes for the Red-Black Tree *must* derive from this
 * class (template). It supplies your class with the necessary members to
 * contain the linking between the tree nodes.
 *
 * @tparam Node    The node class itself. Yes, that's the class derived from
 * this template. This sounds weird, but is correct. See the examples if you're
 * confused.
 * @tparam options  The options class (a version of TreeOptions) that you
 * parameterize the tree with. (See the options parameter of RBTree.)
 * @tparam Tag 		The tag used to identify the tree that this node should
 * be inserted into. See RBTree for details.
 */
template <class Node, class Options = DefaultOptions, class Tag = int>
class WBTreeNodeBase : public bst::BSTNodeBase<Node, Tag> {
public:
	// TODO namespacing!
	void swap_parent_with(Node * other);
	size_t _wbt_size;
};

/**
 * @brief   Helper base class for the NodeTraits you need to implement
 *
 * This class serves as an (optional) base class for the NodeTraits you need to
 * implement. See DOCTODO for details on NodeTraits. This class just implements
 * the various hooks as empty functions.
 *
 * TODO this is the same as the red-black traits - factor it out.
 */
class WBDefaultNodeTraits {
public:
	// TODO document
	template <class Node, class Tree>
	static void
	leaf_inserted(Node & node, Tree & t)
	{
		(void)node;
		(void)t;
	}

	template <class Node, class Tree>
	static void
	rotated_left(Node & node, Tree & t)
	{
		(void)node;
		(void)t;
	}

	template <class Node, class Tree>
	static void
	rotated_right(Node & node, Tree & t)
	{
		(void)node;
		(void)t;
	}

	template <class Node, class Tree>
	static void
	delete_leaf(Node & node, Tree & t)
	{
		(void)node;
		(void)t;
	}

	template <class Node, class Tree>
	static void
	splice_out_left_knee(Node & node, Tree & t)
	{
		(void)node;
		(void)t;
	}

	template <class Node, class Tree>
	static void
	splice_out_right_knee(Node & node, Tree & t)
	{
		(void)node;
		(void)t;
	}

	template <class Node, class Tree>
	static void
	deleted_below(Node & node, Tree & t)
	{
		(void)node;
		(void)t;
	}

	template <class Node, class Tree>
	static void
	swapped(Node & old_ancestor, Node & old_descendant, Tree & t)
	{
		(void)old_ancestor;
		(void)old_descendant;
		(void)t;
	}
};

/**
 * TODO change this
 * @brief The Red-Black Tree
 *
 * This is the main Red-Black Tree class.
 *
 * @tparam Node         The node class for this tree. It must be derived from
 * RBTreeNodeBase.
 * @tparam NodeTraits   A class implementing various hooks and functions on your
 * node class. See DOCTODO for details.
 * @tparam Options			The TreeOptions class specifying the
 * parameters of this RBTree. See the TreeOptions and TreeFlags classes for
 * details.
 * @tparam Tag					An class tag that identifies
 * this tree. Can be used to insert the same nodes into multiple trees. See
 * DOCTODO for details. Can be any class, the class can be empty.
 * @tparam Compare      A compare class. The Red-Black Tree follows STL
 * semantics for 'Compare'. Defaults to ygg::utilities::flexible_less. Implement
 * operator<(const Node & lhs, const Node & rhs) if you want to use it.
 */
template <class Node, class NodeTraits, class Options = DefaultOptions,
          class Tag = int, class Compare = ygg::utilities::flexible_less>
class WBTree : public bst::BinarySearchTree<Node, Options, Tag, Compare> {
public:
	using MyClass = WBTree<Node, NodeTraits, Options, Tag, Compare>;
	// Node Base
	using NB = WBTreeNodeBase<Node, Options, Tag>;
	using TB = bst::BinarySearchTree<Node, Options, Tag, Compare>;
	static_assert(std::is_base_of<NB, Node>::value,
	              "Node class not properly derived from WBTreeNodeBase");

	/**
	 * @brief Create a new empty red-black tree.
	 */
	WBTree();

	/**
	 * @brief Create a new red-black tree from a different red-black tree.
	 *
	 * The other red-black tree is moved into this one, i.e., using it
	 * afterwards is undefined behavior.
	 *
	 * @param other  The red-black tree that this one is constructed from
	 */
	WBTree(MyClass && other);

	/*
	 * Pull in classes from base tree
	 */
	template <bool reverse>
	using iterator = typename TB::template iterator<reverse>;
	template <bool reverse>
	using const_iterator = typename TB::template const_iterator<reverse>;

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
	 * @warning Not available for explicitly ordered trees
	 *
	 * @param   Node  The node to be inserted.
	 */
	void insert(Node & node);

	// TODO document
	void insert_left_leaning(Node & node);
	void insert_right_leaning(Node & node);

	// TODO STL removes *all* elements
	/**
	 * @brief Deletes a node that compares equally to <c> from the tree
	 *
	 * Removes a node that compares equally to <c> from the tree.
	 *
	 * @param   c Anything comparable to a node. A node that compares equally will
	 * be removed
	 */
	template <class Comparable>
	bool erase(const Comparable & c);
	// TODO document
	template <class Comparable>
	void erase_optimistic(const Comparable & c);

	/**
	 * @brief Removes <node> from the tree
	 *
	 * Removes <node> from the tree.
	 *
	 * @param   Node  The node to be removed.
	 */
	void remove(Node & node);

	// Mainly debugging methods
	/// @cond INTERNAL
	bool verify_integrity() const;
	/// @endcond

	/* Debugging methods */
	// TODO only here for compatibility with the Zip Tree
	void dbg_verify() const;
	size_t dbg_count_violations(std::vector<size_t> * depths = nullptr,
	                            std::vector<size_t> * amounts = nullptr) const;
	void dbg_assert_balance_at(Node * n) const;

protected:
	template <bool fix_upward>
	void remove_onepass(Node & node);
	void remove_to_leaf(Node & node);
	void fixup_after_delete(Node * parent, bool deleted_left);

	template <bool call_fixup>
	bool remove_swap_and_remove_left(Node * node, Node * replacement);
	template <bool call_fixup>
	bool remove_swap_and_remove_right(Node * node, Node * replacement);
	template <bool call_fixup>
	void remove_leaf(Node * node);

	template <bool on_equality_prefer_left>
	void insert_leaf_base_twopass(Node & node, Node * start);
	void fixup_after_insert_twopass(Node * node);

	template <bool on_equality_prefer_left>
	void insert_leaf_onepass(Node & node);

	void rotate_left(Node * parent);
	void rotate_right(Node * parent);

	void swap_nodes(Node * n1, Node * n2);
	void replace_node(Node * to_be_replaced, Node * replace_with);
	void swap_unrelated_nodes(Node * n1, Node * n2);
	void swap_neighbors(Node * parent, Node * child);

	void verify_sizes() const;
};

} // namespace ygg

#include "wbtree.cpp"

#endif // YGG_WBTREE_HPP
