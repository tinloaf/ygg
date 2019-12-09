#ifndef YGG_ZIPTREE_H
#define YGG_ZIPTREE_H

#include "bst.hpp"
#include "options.hpp"
#include "size_holder.hpp"
#include "tree_iterator.hpp"
#include "util.hpp"

#ifdef YGG_STORE_SEQUENCE
#include "benchmark_sequence.hpp"
#endif

#include <cmath>
#include <functional>

namespace ygg {

// Forward
template <class Node, class Options = DefaultOptions, class Tag = int>
class ZTreeNodeBase;

namespace ztree_internal {
/// @cond INTERNAL

template <class Tree, bool enable>
struct dbg_verify_size_helper
{
	void operator()(const Tree & t, size_t node_count);
};

template <class Node, class Options, bool use_hash, bool store>
class ZTreeRankGenerator;

template <class Node, class Options>
class ZTreeRankGenerator<Node, Options, true, false> {
public:
	ZTreeRankGenerator();
	static void update_rank(Node & node) noexcept;
	static int get_rank(const Node & node) noexcept;
};

template <class Node, class Options>
class ZTreeRankGenerator<Node, Options, true, true> {
public:
	ZTreeRankGenerator();
	static void update_rank(Node & node) noexcept;
	static size_t get_rank(const Node & node) noexcept;

private:
	template <class, class, class>
	friend class ZTreeNodeBase;
	typename Options::ztree_rank_type::type rank;
};

template <class Node, class Options>
class ZTreeRankGenerator<Node, Options, false, true> {
public:
	ZTreeRankGenerator();
	template <class URBG>
	ZTreeRankGenerator(URBG && g);
	static void update_rank(Node & node) noexcept;
	static size_t get_rank(const Node & node) noexcept;

private:
	template <class, class, class>
	friend class ZTreeNodeBase;
	typename Options::ztree_rank_type::type rank;
};

template <class Node, class Options>
class ZTreeRankGenerator<Node, Options, false, false> {
	// Build a static assertion that always fails, but
	// only if this specialization is ever used. Thus, it must depend on
	// the template parameters.
	static_assert(!std::is_class<Node>::value || std::is_class<Node>::value,
	              "If rank-by-hash is not used, ranks must be stored.");
};
/// @endcond
} // namespace ztree_internal

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
 * parameterize the tree with. (See the options parameter of ZTree.)
 * @tparam Tag 		The tag used to identify the tree that this node should
 * be inserted into. See ZTree for details.
 */
template <class Node, class Options, class Tag>
class ZTreeNodeBase : public bst::BSTNodeBase<Node, Tag> {
public:
	// Debugging methods
	size_t get_depth() const noexcept;

protected:
	/**
	 * @brief Update the stored rank in this node
	 *
	 * If you configure your ZTree to generate the nodes' ranks from hashes (i.e.,
	 * set TreeFlags::ZTREE_USE_HASH) *and* to store the nodes' ranks (i.e., set
	 * TreeFlags::ZTREE_RANK_TYPE), you **must** call this method *before* adding
	 * the node to your tree, but *after* the node's hash has become valid.
	 *
	 * @warning Inserting a node into the tree in the aforementioned case before
	 * calling calling this method leads to undefined behavior.
	 */
	void
	update_rank() noexcept
	{
		ztree_internal::ZTreeRankGenerator<
		    Node, Options, Options::ztree_use_hash,
		    Options::ztree_store_rank>::update_rank(*static_cast<Node *>(this));
	}

private:
	template <class, class, bool, bool>
	friend class ztree_internal::ZTreeRankGenerator;

	ztree_internal::ZTreeRankGenerator<Node, Options, Options::ztree_use_hash,
	                                   Options::ztree_store_rank>
	    _zt_rank;
};

template <class Node>
class ZTreeDefaultNodeTraits {
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

/**
 * @brief The Zip Tree
 *
 * This is the main Zip Tree class.
 *
 * @tparam Node         The node class for this tree. It must be derived from
 * ZTreeNodeBase.
 * @tparam NodeTraits   A class implementing various hooks and functions on your
 * node class. See DOCTODO for details.
 * @tparam Options			The TreeOptions class specifying the
 * parameters of this ZTree. See the TreeOptions and TreeFlags classes for
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

template <
    class Node, class NodeTraits, class Options = DefaultOptions,
    class Tag = int, class Compare = ygg::utilities::flexible_less,
    class RankGetter = ztree_internal::ZTreeRankGenerator<
        Node, Options, Options::ztree_use_hash, Options::ztree_store_rank>>
class ZTree : public bst::BinarySearchTree<Node, Options, Tag, Compare> {
public:
	using NB = ZTreeNodeBase<Node, Options, Tag>;
	using TB = bst::BinarySearchTree<Node, Options, Tag, Compare>;
	using MyClass = ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>;

	/**
	 * @brief Construct a new empty Zip Tree.
	 */
	ZTree() noexcept;

	static_assert(std::is_base_of<NB, Node>::value,
	              "Node class not properly derived from node base!");

	/**
	 * @brief Create a new Zip Tree from a different Zip Tree.
	 *
	 * The other Zip Tree is moved into this one, i.e., using it
	 * afterwards is undefined behavior.
	 *
	 * @param other  The Zip Tree that this one is constructed from
	 */
	ZTree(MyClass && other) noexcept;

	/**
	 * @brief Move-assign an other Zip Tree to this one
	 *
	 * The other Zip Tree is moved into this one, i.e., using it
	 * afterwards is undefined behavior.
	 *
	 * @param other  The Zip Tree that this one is constructed from
	 */
	MyClass & operator=(MyClass && other) noexcept;

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
	 * For zip trees, the hinted version is equivalent to the unhinted insertion.
	 *
	 * @param   Node  The node to be inserted.
	 */
	void insert(Node & node) noexcept;
	void insert(Node & node, Node & hint) noexcept;

	/**
	 * @brief Removes <node> from the tree
	 *
	 * Removes <node> from the tree.
	 *
	 * @param   Node  The node to be removed.
	 */
	void remove(Node & node) CMP_NOEXCEPT(node);

	// TODO STL removes *all* elements
	/**
	 * @brief Deletes a node that compares equally to <c> from the tree
	 *
	 * Removes a node that compares equally to <c> from the tree.
	 *
	 * @param   c Anything comparable to a node. A node that compares equally will
	 * be removed
	 * @return A pointer to the node that was removed, if any, otherwise nullptr.
	 */
	template <class Comparable>
	utilities::select_type_t<size_t, Node *, Options::stl_erase>
	erase(const Comparable & c) CMP_NOEXCEPT(c);

	template <bool reverse>
	utilities::select_type_t<const iterator<reverse>, Node *, Options::stl_erase>
	erase(const iterator<reverse> & it) CMP_NOEXCEPT(*it);

	// Debugging methods
	void dbg_verify() const;
	void dbg_print_rank_stats() const;

	/**
	 * @brief Debugging Method: Draw the Tree as a .dot file
	 *
	 * Outputs the current tree as a .dot file which can be drawn using
	 * graphviz.
	 *
	 * @param filename  The file path where to write the .dot file.
	 */
	void dump_to_dot(const std::string & filename) const;

private:
	void unzip(Node & oldn, Node & newn) noexcept;
	void zip(Node & old_root) noexcept;

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

#ifdef YGG_STORE_SEQUENCE
	typename ygg::utilities::template BenchmarkSequenceStorage<
	    typename Options::SequenceInterface::KeyT>
	    bss;
#endif
};

} // namespace ygg

#ifndef YGG_ZIPTREE_CPP
#include "ziptree.cpp"
#endif

#endif
