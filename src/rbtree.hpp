#ifndef YGG_RBTREE_HPP
#define YGG_RBTREE_HPP

#include "bst.hpp"
#include "options.hpp"
#include "size_holder.hpp"
#include "tree_iterator.hpp"

#include <cassert>
#include <cstddef>
#include <set>
#include <type_traits>

// Only for debugging purposes
#include <fstream>
#include <vector>

namespace ygg {
namespace rbtree_internal {
/// @cond INTERNAL

enum class Color : unsigned char
{
	BLACK = 0,
	RED = 1
};

template <class Node, bool compress_color>
class ColorParentStorage;

template <class Node>
class ColorParentStorage<Node, true> {
public:
	void set_color(Color new_color) noexcept;
	void make_black() noexcept;
	void make_red() noexcept;

	Color get_color() const noexcept;
	void set_parent(Node * new_parent) noexcept;
	Node * get_parent() const noexcept;

	void swap_parent_with(ColorParentStorage<Node, true> & other) noexcept;
	void swap_color_with(ColorParentStorage<Node, true> & other) noexcept;

	static constexpr bool parent_reference = false;

private:
	Node * parent;
};

template <class Node>
class ColorParentStorage<Node, false> {
public:
	void set_color(Color new_color) noexcept;
	void make_black() noexcept;
	void make_red() noexcept;

	Color get_color() const noexcept;
	void set_parent(Node * new_parent) noexcept;
	Node *& get_parent() noexcept;
	Node * get_parent() const noexcept;

	void swap_parent_with(ColorParentStorage<Node, false> & other) noexcept;
	void swap_color_with(ColorParentStorage<Node, false> & other) noexcept;

	static constexpr bool parent_reference = true;

private:
	Node * parent = nullptr;
	Color color;
};

/// @endcond
} // namespace rbtree_internal

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
class RBTreeNodeBase
    : public bst::BSTNodeBase<
          Node, Options, Tag,
          rbtree_internal::ColorParentStorage<Node, Options::compress_color>> {
public:
	// TODO namespacing!

	void set_color(rbtree_internal::Color new_color) noexcept;
	void make_black() noexcept;
	void make_red() noexcept;
	rbtree_internal::Color get_color() const noexcept;

	void swap_parent_with(Node * other) noexcept;
	void swap_color_with(Node * other) noexcept;

private:
	using ActiveOptions = Options;
	friend class rbtree_internal::ColorParentStorage<Node,
	                                                 Options::compress_color>;
};

/**
 * @brief   Helper base class for the NodeTraits you need to implement
 *
 * This class serves as an (optional) base class for the NodeTraits you need to
 * implement. See DOCTODO for details on NodeTraits. This class just implements
 * the various hooks as empty functions.
 */
class RBDefaultNodeTraits {
public:
	// TODO document
	template <class Node, class Tree>
	static void
	leaf_inserted(Node & node, Tree & t) noexcept
	{
		(void)node;
		(void)t;
	}

	template <class Node, class Tree>
	static void
	rotated_left(Node & node, Tree & t) noexcept
	{
		(void)node;
		(void)t;
	}

	template <class Node, class Tree>
	static void
	rotated_right(Node & node, Tree & t) noexcept
	{
		(void)node;
		(void)t;
	}

	template <class Node, class Tree>
	static void
	delete_leaf(Node & node, Tree & t) noexcept
	{
		(void)node;
		(void)t;
	}

	template <class Node, class Tree>
	static void
	deleted_below(Node & node, Tree & t) noexcept
	{
		(void)node;
		(void)t;
	}

	template <class Node, class Tree>
	static void
	swapped(Node & old_ancestor, Node & old_descendant, Tree & t) noexcept
	{
		(void)old_ancestor;
		(void)old_descendant;
		(void)t;
	}
};

/**
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
class RBTree
    : public bst::BinarySearchTree<
          Node, Options, Tag, Compare,
          rbtree_internal::ColorParentStorage<Node, Options::compress_color>>

{
public:
	using MyClass = RBTree<Node, NodeTraits, Options, Tag, Compare>;
	// Node Base
	using NB = RBTreeNodeBase<Node, Options, Tag>;
	using TB = bst::BinarySearchTree<
	    Node, Options, Tag, Compare,
	    rbtree_internal::ColorParentStorage<Node, Options::compress_color>>;
	static_assert(std::is_base_of<NB, Node>::value,
	              "Node class not properly derived from RBTreeNodeBase");

	/**
	 * @brief Create a new empty red-black tree.
	 */
	RBTree() noexcept;

	/**
	 * @brief Create a new red-black tree from a different red-black tree.
	 *
	 * The other red-black tree is moved into this one, i.e., using it
	 * afterwards is undefined behavior.
	 *
	 * @param other  The red-black tree that this one is constructed from
	 */
	RBTree(MyClass && other) noexcept;

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
	 * @return An iterator to the inserted node if insertion was successfull, an
	 *         iterator to the blocking node otherwise.
	 */
	iterator<false> insert(Node & node) CMP_NOEXCEPT(node);
	iterator<false> insert(Node & node, Node & hint) CMP_NOEXCEPT(node);
	iterator<false> insert(Node & node, iterator<false> hint) CMP_NOEXCEPT(node);

	// TODO document hinted inserts
	// TODO should order be preserved on hints?

	/**
	 * @brief Removes <node> from the tree
	 *
	 * Removes <node> from the tree.
	 *
	 * @param   Node  The node to be removed.
	 */
	void remove(Node & node) CMP_NOEXCEPT(node);

	/**
	 * @brief Deletes a node that compares equally to <c> from the tree
	 *
	 * @warning The behavior of this method strongly depends on whether the
	 * STL_ERASE option is set or not!
	 *
	 * Removes a node that compares equally to <c> from the tree.
	 *
	 * If STL_ERASE is set, this method removes *all* nodes that compare equally
	 * to c from the tree and returns the number of nodes removed.
	 *
	 * If STL_ERASE is not set, it removes only one node and returns a pointer to
	 * the removed node.
	 *
	 * @param  c Anything comparable to a node. A node (resp. all nodes, see
	 * above) that compares equally will be removed
	 *
	 * @return If STL_ERASE is unset: A pointer to the node that has been removed,
	 * or nullptr if no node was removed. If STL_ERASE is set: The number of
	 * erased nodes.
	 */
	template <class Comparable>
	utilities::select_type_t<size_t, Node *, Options::stl_erase>
	erase(const Comparable & c) CMP_NOEXCEPT(c);

	/**
	 * @brief Deletes a node by iterator
	 *
	 * @warning The return type of this method depends on whether the
	 * STL_ERASE option is set or not!
	 *
	 * Removes a node that is pointed to by an iterator.
	 *
	 * @param  c Anything comparable to a node. A node (resp. all nodes, see
	 * above) that compares equally will be removed
	 *
	 * @return If STL_ERASE is unset: A pointer to the node that has been removed.
	 * If STL_ERASE is set: An iterator to the node after the removed node (or
	 * end()).
	 */
	template <bool reverse>
	utilities::select_type_t<const iterator<reverse>, Node *, Options::stl_erase>
	erase(const iterator<reverse> & it) CMP_NOEXCEPT(*it);

	// Mainly debugging methods
	/// @cond INTERNAL
	void dbg_verify() const;
	bool verify_integrity() const;
	/// @endcond

protected:
	using Path = std::vector<Node *>;

	void remove_to_leaf(Node & node) CMP_NOEXCEPT(node);
	void fixup_after_delete(Node * parent, bool deleted_left) noexcept;

	iterator<false> insert_leaf_base(Node & node, Node * start)
	    CMP_NOEXCEPT(node);

	void fixup_after_insert(Node * node) noexcept;
	void rotate_left(Node * parent) noexcept;
	void rotate_right(Node * parent) noexcept;

	void swap_nodes(Node * n1, Node * n2, bool swap_colors = true) noexcept;
	void replace_node(Node * to_be_replaced, Node * replace_with) noexcept;
	void swap_unrelated_nodes(Node * n1, Node * n2) noexcept;
	void swap_neighbors(Node * parent, Node * child) noexcept;

	void verify_black_root() const;
	void verify_black_paths(const Node * node, unsigned int * path_length) const;
	void verify_red_black(const Node * node) const;
};

} // namespace ygg

#ifndef YGG_RBTREE_CPP
#include "rbtree.cpp"
#endif

#endif // YGG_RBTREE_HPP
