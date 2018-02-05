//
// Created by lukas on 10.11.17.
//

#ifndef YGG_DYNAMIC_SEGMENT_TREE_HPP
#define YGG_DYNAMIC_SEGMENT_TREE_HPP

#include "rbtree.hpp"
#include "options.hpp"
#include "size_holder.hpp"
#include "util.hpp"
#include "debug.hpp"

namespace ygg {

namespace dyn_segtree_internal {
/// @cond INTERNAL
template<class Tag>
class InnerRBTTag {};

template<class KeyT_in, class ValueT_in, class AggValueT_in, class Combiners, class Tag>
class InnerNode : public RBTreeNodeBase<InnerNode<KeyT_in, ValueT_in, AggValueT_in, Combiners, Tag>,
                                        TreeOptions<TreeFlags::MULTIPLE>, InnerRBTTag<Tag>>
{
public:
	using KeyT = KeyT_in;
	using ValueT = ValueT_in;
	using AggValueT = AggValueT_in;

	KeyT point;
	bool start;

	InnerNode<KeyT, ValueT, AggValueT, Combiners, Tag> * partner;

	ValueT val; // TODO this can be removed!
	AggValueT agg_left;
	AggValueT agg_right;

	Combiners combiners;
};

template<class InnerTree, class InnerNode>
class InnerNodeTraits : public RBDefaultNodeTraits<InnerNode> {
public:
	static void leaf_inserted(InnerNode & node);
	static void rotated_left(InnerNode & node);
	static void rotated_right(InnerNode & node);
	static void delete_leaf(InnerNode & node);
	static void swapped(InnerNode & n1, InnerNode & n2);
};

template<class InnerNode>
class Compare {
public:
	bool operator()(const InnerNode & lhs,
	                const InnerNode & rhs) const
	{
		if (lhs.point != rhs.point) {
			return lhs.point < rhs.point;
		} else {
			return (!lhs.start && rhs.start);
		}
	}

	bool operator()(typename InnerNode::KeyT lhs, const InnerNode & rhs) const
	{
		return lhs < rhs.point;
	}

	bool operator()(const InnerNode & lhs, typename InnerNode::KeyT rhs) const
	{
		return lhs.point < rhs;
	}
};

// TODO Debug
template<class InnerNode, class ... Combiners>
class InnerNodeNameGetter {
public:
	InnerNodeNameGetter() {};

	std::string get_name(InnerNode * node) const {
		return std::string("[") + std::to_string(node->point) + std::string("]")
		       + std::string("@") + std::to_string((unsigned long)node)
						+ std::string("  ╭:") + std::to_string(node->agg_left) + std::string("  ╮:")
						+ std::to_string(node->agg_right) + std::string("  {")
						+ std::string {
								Combiners::get_name() + std::string(": ")
								+ std::to_string(node->combiners.template get<Combiners>()) + std::string(" ")
								...
							}
						+ std::string("}");
	}
};

/// @endcond
} // namespace internal

template<class ValueType>
class MaxCombiner {
public:
	using ValueT = ValueType;

	explicit MaxCombiner(ValueT val);
	MaxCombiner() = default;

	// TODO the bool is only returned for sake of expansion! Fix that!
	bool combine_with(ValueT a, ValueT edge_val);
	bool rebuild(ValueT a, ValueT a_edge_val, ValueT b, ValueT b_edge_val);

	ValueT get();

	// TODO DEBUG
	static std::string get_name() {
		return "MaxCombiner";
	}
private:
	ValueT val;
};

template<class AggValueT, class ... Combiners>
class CombinerPack {
public:
	explicit CombinerPack(AggValueT val);
	CombinerPack() = default;

	bool rebuild(CombinerPack<AggValueT, Combiners...> * a,
	             AggValueT a_edge_val,
	             CombinerPack<AggValueT, Combiners...> * b,
	             AggValueT b_edge_val);

	// TODO the bool is only returned for sake of expansion! Fix that!
	bool combine_with(CombinerPack<AggValueT, Combiners...> * other, AggValueT edge_val);

	template<class Combiner>
	typename Combiner::ValueT get();

	using pack = utilities::pack<Combiners ...>;
private:
	std::tuple<Combiners ...> data;
};

template<class AggValueT>
using EmptyCombinerPack = CombinerPack<AggValueT>;

/**
 * @brief Base class (template) to supply your node class with metainformation
 *
 * The class you use as nodes for the Dynamic Segment Tree *must* derive from this
 * class (template). It supplies your class with the necessary members to
 * contain the linking between the tree nodes.
 *
 * @tparam KeyType				The type of the key, i.e., the interval borders
 * @tparam ValueType 			The type of the values that every interval is associated with
 * @tparam AggValueType		The typo of an aggregate of multiple ValueT_in's. See DOCTODO for details.
 * @tparam Tag 						The tag used to identify the tree that this node should be inserted into. See
 * RBTree for details.
 */
template<class KeyType, class ValueType, class AggValueType, class Combiners, class Tag = int>
class DynSegTreeNodeBase {
public:
	/// @cond INTERNAL
	using KeyT = KeyType;
	using ValueT = ValueType;
	using AggValueT = AggValueType;

	using InnerNode = dyn_segtree_internal::InnerNode<KeyT, ValueT, AggValueT, Combiners, Tag>;

	InnerNode start;
	InnerNode end;
	/// @endcond
};


/**
 * @brief You must derive your own traits class from this class template, telling the
 * DynamicSegmentTree how to interact with your node class.
 *
 * You must derive from this class template and supply the DynamicSegmentTree with your own derived
 * class. At the least, you have to implement the methods get_lower, get_upper and get_value for
 * the DynamicSegmentTree to work. See the respective methods' documentation for details.
 *
 * @tparam Node 	Your node class to be used in the DynamicSegmentTree, derived from DynSegTreeNodeBase
 */
template<class Node>
class DynSegTreeNodeTraits {
public:
	/**
	 * The type of the borders of intervals / segments in the DynamicSegmentTree
	 */
	using KeyT = typename Node::KeyT;
	/**
	 * The type of the values associated with the intervals in the DynamicSegmentTree
	 */
	using ValueT = typename Node::ValueT;

	/**
	 * Must be implemented to return the lower bound of the interval represented by n.
	 *
	 * @param n The node whose lower interval bound should be returned.
	 * @return Must return the lower interval bound of n
	 */
	static KeyT get_lower(const Node & n);

	/**
	 * Must be implemented to return the upper bound of the interval represented by n.
	 *
	 * @param n The node whose upper interval bound should be returned.
	 * @return Must return the upper interval bound of n
	 */
	static KeyT get_upper(const Node & n);

	/**
	 * Must be implemented to return the value associated with the interval represented by n.
	 *
	 * @param n The node whose associated value should be returned
	 * @return Must return the value associated with n
	 */
	static ValueT get_value(const Node & n);
};

/**
 * @brief The Dynamic Segment Tree class
 *
 * This class provides a dynamic version of a segment tree. For details on the implementation,
 * see DOCTODO. The dynamic segment tree provides the following operations:
 *
 * * Querying for the aggregate value ta a point x (a "stabbing query") in O(log n) * A
 * * Insertion of a new interval in O(log n) * A
 * * Deletion of an interval in O(log n) * A
 *
 * where n is the number of intervals in the dynamic segment tree and A is the time it takes to
 * aggregate a value, i.e., compute operator+(AggValueT, ValueT).
 *
 * DOCTODO aggregators
 *
 * @tparam Node					The node class in your tree, must be derived from DynSegTreeNodeBase
 * @tparam NodeTraits		The node traits for your node class, must be derived from DynSegTreeNodeTraits
 * @tparam Options			Options for this tree. See DOCTODO for details.
 * @tparam Tag					The tag of this tree. Allows to insert the same node in multiple dynamic
 * 											segment trees. See DOCTODO for details.
 */
// TODO DOC right-open intervals

// TODO constant-time size
template <class Node, class NodeTraits, class Combiners, class Options = DefaultOptions,
					class Tag = int>
class DynamicSegmentTree
{
private:
	using NB = DynSegTreeNodeBase<typename Node::KeyT, typename Node::ValueT,
	                        typename Node::AggValueT, Combiners, Tag>;
	using InnerNode = typename NB::InnerNode;

	static_assert(std::is_base_of<DynSegTreeNodeTraits<Node>, NodeTraits>::value,
	             "NodeTraits not properly derived from DynSegTreeNodeTraits!");
	static_assert(std::is_base_of<NB, Node>::value,
	              "Node class not properly derived from DynSegTreeNodeBase!");
	static_assert(Options::multiple, "DynamicSegmentTree always allows multiple equal intervals.");

public:
	using KeyT = typename Node::KeyT;
	using ValueT = typename Node::ValueT;
	using AggValueT = typename Node::AggValueT;

private:
	class InnerTree : public RBTree<InnerNode, dyn_segtree_internal::InnerNodeTraits<InnerTree, InnerNode>,
	                          TreeOptions<TreeFlags::MULTIPLE>,
	                          dyn_segtree_internal::InnerRBTTag<Tag>, dyn_segtree_internal::Compare<InnerNode>>
	{
	public:
		using BaseTree = RBTree<InnerNode, dyn_segtree_internal::InnerNodeTraits<InnerTree, InnerNode>,
		                        TreeOptions<TreeFlags::MULTIPLE>,
		                        dyn_segtree_internal::InnerRBTTag<Tag>, dyn_segtree_internal::Compare<InnerNode>>;

		using BaseTree::BaseTree;

		static void modify_contour(InnerNode * left, InnerNode * right, ValueT val);

		using Contour = std::pair<std::vector<InnerNode *>, std::vector<InnerNode *>>;
		static Contour find_lca(InnerNode * left, InnerNode * right);

		static bool rebuild_combiners_at(InnerNode * n);
		static void rebuild_combiners_recursively(InnerNode * n);
	};

public:

	/**
	 * @brief Insert an interval into the dynamic segment tree
	 *
	 * This inserts the interval represented by the node n into the dynamic segment tree.
	 * The interval may not be empty.
	 *
	 * @param n		The node representing the interval being inserted
	 */
	void insert(Node &n);

	/**
	 * @brief Removes an intervals from the dynamic segment tree
	 *
	 * Removes the (previously inserted) node n from the dynamic segment tree
	 *
	 * @param n 	The node to be removed
	 */
	void remove(Node &n);

	/**
	 * @brief Returns whether the dynamic segment tree is empty
	 *
	 * This method runs in O(1).
	 *
	 * @return true if the dynamic segment tree is empty, false otherwise
	 */
	bool empty() const;

	/**
	 * @brief Perform a stabbing query at point x
	 *
	 * This query asks for the aggregate value over all intervals containing point x. This is a
	 * "stabbing query".
	 *
	 * @param 		x The point to query for
	 * @return 		The aggregated value for all intervals containing x
	 */
	AggValueT query(const typename Node::KeyT & x);

	template<class Combiner>
	typename Combiner::ValueT get_combined() const;

	template<class Combiner>
	typename Combiner::ValueT get_combined(const typename Node::KeyT & lower,
	                                       const typename Node::KeyT & upper) const;

	/*
	 * DEBUGGING
	 */
private:
	// TODO build a generic function for this
	template<class ... Ts>
	using NodeNameGetterCurried = dyn_segtree_internal::InnerNodeNameGetter<InnerNode, Ts ...>;
	using NodeNameGetter = typename utilities::pass_pack<typename Combiners::pack,
	                                                     NodeNameGetterCurried>::type;
public:
	using TreePrinter = debug::TreePrinter<InnerNode, NodeNameGetter>;

private:
	void apply_interval(Node & n);
	void unapply_interval(Node & n);

	InnerTree t;
};

} // namespace ygg

#include "dynamic_segment_tree.cpp"

#endif //YGG_DYNAMIC_SEGMENT_TREE_HPP
