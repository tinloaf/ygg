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

/**
 * @brief An inner node, representing either a start or an end of an interval
 */
template<class KeyT_in, class ValueT_in, class AggValueT_in, class Combiners, class Tag>
class InnerNode : public RBTreeNodeBase<InnerNode<KeyT_in, ValueT_in, AggValueT_in, Combiners, Tag>,
                                        TreeOptions<TreeFlags::MULTIPLE>, InnerRBTTag<Tag>>
{
public:
	using KeyT = KeyT_in;
	using ValueT = ValueT_in;
	using AggValueT = AggValueT_in;

	// TODO instead of storing all of these, store a pointer to the containing interval and use
	// interval traits?
	KeyT point;
	bool start;
	bool closed;

	// TODO remove this
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
	using PointDescription = std::pair<const typename InnerNode::KeyT, const int_fast8_t>;

	bool operator()(const InnerNode & lhs,
	                const InnerNode & rhs) const
	{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
		if (lhs.point != rhs.point) {
#pragma GCC diagnostic pop
			return lhs.point < rhs.point;
		} else {
			/*
			 * At the same point, the order is: open ends, closed starts, closed ends, open starts
			 */
			int_fast8_t lhs_priority;
			if (lhs.closed) {
				if (lhs.start) {
					lhs_priority = -1;
				} else {
					lhs_priority = 1;
				}
			} else {
				if (lhs.start) {
					lhs_priority = 2;
				} else {
					lhs_priority = -2;
				}
			}

			int_fast8_t rhs_priority;
			if (rhs.closed) {
				if (rhs.start) {
					rhs_priority = -1;
				} else {
					rhs_priority = 1;
				}
			} else {
				if (rhs.start) {
					rhs_priority = 2;
				} else {
					rhs_priority = -2;
				}
			}

			return lhs_priority < rhs_priority;
		}
	}

	bool operator()(const typename InnerNode::KeyT & lhs, const InnerNode & rhs) const
	{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
		if (lhs != rhs.point) {
			return lhs < rhs.point;
		}
#pragma GCC diagnostic pop

		// only open starts are strictly larger than the key
		return (rhs.start && !rhs.closed);
	}

	bool operator()(const InnerNode & lhs, const typename InnerNode::KeyT & rhs) const
	{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
		if (lhs.point != rhs) {
			return lhs.point < rhs;
		}
#pragma GCC diagnostic pop

		// only open ends must strictly go before the key
		return (!lhs.start && !lhs.closed);
	}


	bool operator()(const PointDescription & lhs, const InnerNode & rhs) const
	{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
		if (lhs.first != rhs.point) {
			return lhs.first < rhs.point;
		}
#pragma GCC diagnostic pop

		if (lhs.second > 0) {
			// the query is left-open, i.e., it should not to before anything
			return false;
		} else if (lhs.second < 0) {
			// the query is right-open. It should go before everything but open end nodes
			return rhs.start || rhs.closed;
		} else {
			// The query is closed.
			// only open starts are strictly larger than the key
			return (rhs.start && !rhs.closed);
		}
	}

	bool operator()(const InnerNode & lhs, const PointDescription & rhs) const
	{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
		if (lhs.point != rhs.first) {
			return lhs.point < rhs.first;
		}
#pragma GCC diagnostic pop
		
		if (rhs.second > 0) {
			// the query is left-open, i.e., everything but left-open is before it
			return !(lhs.start && !lhs.closed);
		} else if (rhs.second > 0) {
			// the query is right-open, i.e., nothing must ever strictly go before it
			return false;
		} else {
			// the query is closed
			// only open ends must strictly go before the key
			return (!lhs.start && !lhs.closed);
		}
	}
};

// TODO Debug
template<class InnerNode, class ... Combiners>
class ASCIIInnerNodeNameGetter {
public:
	ASCIIInnerNodeNameGetter() {};

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

template<class InnerNode, class ... Combiners>
class DOTInnerNodeNameGetter {
public:
	DOTInnerNodeNameGetter() {};

	std::string get_name(InnerNode * node) const {
		std::stringstream name;

		std::string combiner_str {
						Combiners::get_name() + std::string(": ")
						+ std::to_string(node->combiners.template get<Combiners>()) + std::string(" ")
										...
		};

		if (node->start) {
			if (node->closed) {
				name << "[";
			} else {
				name << "(";
			}
		}
		name << node->point;
		if (!node->start) {
			if (node->closed) {
				name << "]";
			} else {
				name << ")";
			}
		}

		name << " @" << node->val;
		name << "\\n {->" << node->partner->point << "} \\n<";
		name << combiner_str << ">";

		return name.str();
	}
};

template<class InnerNode>
class DOTInnerEdgeNameGetter {
public:
	DOTInnerEdgeNameGetter() {};

	std::string get_name(InnerNode * node, bool left) const {
		if (left) {
			return std::to_string(node->agg_left);
		} else {
			return std::to_string(node->agg_right);
		}
	}
};

/// @endcond
} // namespace internal

/**
 * @brief A combiner that allows to retrieve the maximum value over any range
 *
 * This is a combiner (see TODO for what a combiner is) that, when added to a Dynamic Segment
 * Tree, allows you to efficiently retrieve the maximum aggregate value over any range in your
 * segment tree.
 *
 * @tparam ValueType The type of values associated with your intervals
 */
template<class ValueType>
class MaxCombiner {
public:
	using ValueT = ValueType;

	/**
	 * @brief Build this combiner from just this node, i.e., as if it didn't have any children
	 *
	 * @param val The value associated with this node TODO CAN BE REMOVED
	 */
	explicit MaxCombiner(ValueT val);

	MaxCombiner() = default;

	// TODO the bool is only returned for sake of expansion! Fix that!
	/**
	 * @brief Combines this MaxCombiner with a value, possibly of a child node
	 *
	 * This sets the maximum currently stored at this combiner to the maximum of the currently
	 * stored value and (a + edge_val).
	 *
	 * Usually, a will be the value of the MaxCombiner of a child of the node that this combiner
	 * belongs to. edge_val will then be the agg_left or agg_right
	 * value of the node this combiner belongs to.
	 *
	 * @param a 				See above
	 * @param edge_val 	See above
	 * @return FIXME ignored for now
	 */
	bool combine_with(ValueT a, ValueT edge_val);

	// TODO the bool is only returned for sake of expansion! Fix that!
	/**
	 * @brief Aggregates a value into the max value stored in this combiner
	 *
	 * This sets the maximum currently stored at this combiner to the maximum of the currently
	 * stored value and (a + edge_val).
	 *
	 * Usually, a will be the value of the MaxCombiner of a child of the node that this combiner
	 * belongs to. edge_val will then be the agg_left or agg_right
	 * value of the node this combiner belongs to.
	 *
	 * @param a 				See above
	 * @param edge_val 	See above
	 * @return FIXME ignored for now
	 */
	 bool aggregate_with(ValueT a);

	/**
	 * @brief Rebuilds the value in this MaxCombiner from values of its two children's MaxCombiners
	 *
	 * This sets the maximum currently stored at this combiner to the maximum of (a + a_edge_val) and
	 * (b + b_edge_val).
	 *
	 * Usually, a and b will be the values of the MaxCombiners of the two children of the node that
	 * this  combiner belongs to. a_edge_val  and b_edge_val will then be the agg_left resp. agg_right
	 * values of the node this combiner belongs to.
	 *
	 * @param a 				See above
	 * @param a_edge_val 	See above
	 * @param b 				See above
	 * @param b_edge_val 	See above
	 * @return FIXME ignored for now
	 */
	bool rebuild(ValueT a, ValueT a_edge_val, ValueT b, ValueT b_edge_val);

	/**
	 * @brief Returns the currently stored combined value in this combiner
	 *
	 * @return the currently stored combined value in this combiner
	 */
	ValueT get();

	// TODO DEBUG
	static std::string get_name() {
		return "MaxCombiner";
	}
private:
	ValueT val;
};

/**
 * @brief This class represents the pack of combiners associated with every node of a Dynamic
 * Segment Tree
 *
 * A DynamicSegmentTree can have multiple combiners associated with each node. See TODO for
 * details. Every combiner allows to retrieve a different combined metric (such as maximum,
 * minimum, …) of the aggregate values over arbitrary ranges in the Dynamic Segment Tree.
 *
 * @tparam AggValueT	The type of the aggregate values in your DynamicSegmentTree
 * @tparam Combiners	A list of combiner classes
 */
template<class AggValueT, class ... Combiners>
class CombinerPack {
public:
	/**
	 * @brief Initialize all combiners at this node as if this node did not have any children
	 * @param val TODO CAN BE REMOVED
	 */
	explicit CombinerPack(AggValueT val);
	CombinerPack() = default;

	/**
	 * @brief Rebuilds all combiners at this node from its children's combiners
	 *
	 * This method calls the rebuild() method on all combiners attached to this node with the
	 * respective combined values from the left / right child.
	 *
	 * @param a						The combiner pack of our node's left child
	 * @param a_edge_val	The agg_left value of our node
	 * @param b						The combiner pack of our node's right child
	 * @param b_edge_val	The agg_right value of our node
	 * @return TODO IGNORED
	 */
	bool rebuild(CombinerPack<AggValueT, Combiners...> * a,
	             AggValueT a_edge_val,
	             CombinerPack<AggValueT, Combiners...> * b,
	             AggValueT b_edge_val);

	// TODO the bool is only returned for sake of expansion! Fix that!
	bool combine_with(CombinerPack<AggValueT, Combiners...> * other, AggValueT edge_val);
	// TODO the bool is only returned for sake of expansion! Fix that!
	bool aggregate_with(AggValueT a);

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
	// TODO why is all of this public?
public:
	/// @cond INTERNAL
	using KeyT = KeyType;
	using ValueT = ValueType;
	using AggValueT = AggValueType;

	using InnerNode = dyn_segtree_internal::InnerNode<KeyT, ValueT, AggValueT, Combiners, Tag>;

	// TODO make these private
	/**
	 * @brief RBTree node that represents the start of the interval represented by this
	 * DynSegTreeNodeBase
	 */
	InnerNode start;

	/**
	 * @brief RBTree node that represents the end of the interval represented by this
	 * DynSegTreeNodeBase
	 */
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
	 * Should be implemented to indicate whether an interval contains its lower border or not.
	 *
	 * The default (if this method is not implemented) is true.
	 */
	static bool is_lower_closed(const Node & n) { (void)n; return true; };

	/**
	 * Should be implemented to indicate whether an interval contains its upper border or not.
	 *
	 * The default (if this method is not implemented) is false.
	 */
	static bool is_upper_closed(const Node & n) { (void)n; return false; };

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
 * DOCTODO combiners
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
	                                       const typename Node::KeyT & upper,
																				 bool lower_closed = true,
																			   bool upper_closed = false) const;

	/*
	 * DEBUGGING
	 */
private:
	// TODO build a generic function for this
	template<class ... Ts>
	using NodeNameGetterCurried = dyn_segtree_internal::ASCIIInnerNodeNameGetter<InnerNode, Ts ...>;
	using NodeNameGetter = typename utilities::pass_pack<typename Combiners::pack,
	                                                     NodeNameGetterCurried>::type;
	template<class ... Ts>
	using DotNameGetterCurried = dyn_segtree_internal::DOTInnerNodeNameGetter<InnerNode, Ts...>;
	using DotNameGetter = typename utilities::pass_pack<typename Combiners::pack,
	                                                    DotNameGetterCurried>::type;

	using TreePrinter = debug::TreePrinter<InnerNode, NodeNameGetter>;
	using TreeDotExporter = debug::TreeDotExport<InnerNode,
	                                             DotNameGetter,
	                                             dyn_segtree_internal::DOTInnerEdgeNameGetter<InnerNode>>;
public:
	// TODO Debugging only!
	void dbg_print_inner_tree() const;
	std::stringstream & dbg_get_dot() const;

private:
	void apply_interval(Node & n);
	void unapply_interval(Node & n);

	InnerTree t;
};

} // namespace ygg

#include "dynamic_segment_tree.cpp"

#endif //YGG_DYNAMIC_SEGMENT_TREE_HPP
