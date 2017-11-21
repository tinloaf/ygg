//
// Created by lukas on 10.11.17.
//

#ifndef YGG_DYNAMIC_SEGMENT_TREE_HPP
#define YGG_DYNAMIC_SEGMENT_TREE_HPP

#include "rbtree.hpp"
#include "options.hpp"
#include "size_holder.hpp"

namespace ygg {

namespace dyn_segtree_internal {
/// @cond INTERNAL
template<class Tag>
class InnerRBTTag {};

template<class KeyT_in, class ValueT_in, class AggValueT_in, class Tag>
class InnerNode : public RBTreeNodeBase<InnerNode<KeyT_in, ValueT_in, AggValueT_in, Tag>,
                                        TreeOptions<TreeFlags::MULTIPLE>, InnerRBTTag<Tag>>
{
public:
	using KeyT = KeyT_in;
	using ValueT = ValueT_in;
	using AggValueT = AggValueT_in;

	KeyT point;
	bool start;

	AggValueT agg_left;
	AggValueT agg_right;
};

template<class InnerTree, class InnerNode>
class InnerNodeTraits : public RBDefaultNodeTraits<InnerNode> {
public:
	static void leaf_inserted(InnerNode & node);
	static void rotated_left(InnerNode & node);
	static void rotated_right(InnerNode & node);
	static void deleted_below(InnerNode & node) {};
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
		return lhs.point <= rhs;
	}
};

// TODO Debug
template<class InnerNode>
class InnerNodeNameGetter {
public:
	InnerNodeNameGetter() {};

	std::string get_name(InnerNode * node) const {
		return std::string("[") + std::to_string(node->point) + std::string("]")
						+ std::string("  ╭:") + std::to_string(node->agg_left) + std::string("  ╮:")
						+ std::to_string(node->agg_right);
	}
};

/// @endcond
} // namespace internal

template<class KeyT_in, class ValueT_in, class AggValueT_in, class Tag = int>
class DynSegTreeNodeBase {
public:
	/// @cond INTERNAL
	using KeyT = KeyT_in;
	using ValueT = ValueT_in;
	using AggValueT = AggValueT_in;

	using InnerNode = dyn_segtree_internal::InnerNode<KeyT, ValueT, AggValueT, Tag>;

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
 * @tparam Node 	Your node class to be used in the DynamicSegmentTree
 */
template<class Node>
class DynSegTreeNodeTraits {
public:
	/**
	 * The type of the keys of intervals / segments in the DynamicSegmentTree
	 */
	using KeyT = typename Node::KeyT;
	/**
	 * The type of the values / aggregates in the DynamicSegmentTree
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

template <class Node, class NodeTraits, class Options = DefaultOptions, class Tag = int>
class DynamicSegmentTree
{
public:
	using NB = DynSegTreeNodeBase<typename Node::KeyT, typename Node::ValueT,
	                        typename Node::AggValueT, Tag>;
	using InnerNode = typename NB::InnerNode;

	static_assert(std::is_base_of<DynSegTreeNodeTraits<Node>, NodeTraits>::value,
	             "NodeTraits not properly derived from DynSegTreeNodeTraits!");
	static_assert(std::is_base_of<NB, Node>::value,
	              "Node class not properly derived from DynSegTreeNodeBase!");
	static_assert(Options::multiple, "DynamicSegmentTree always allows multiple equal intervals.");

	class Tree : public RBTree<InnerNode, dyn_segtree_internal::InnerNodeTraits<Tree, InnerNode>,
	                          TreeOptions<TreeFlags::MULTIPLE>,
	                          dyn_segtree_internal::InnerRBTTag<Tag>, dyn_segtree_internal::Compare<InnerNode>>
	{
	public:
		using BaseTree = RBTree<InnerNode, dyn_segtree_internal::InnerNodeTraits<Tree, InnerNode>,
		                        TreeOptions<TreeFlags::MULTIPLE>,
		                        dyn_segtree_internal::InnerRBTTag<Tag>, dyn_segtree_internal::Compare<InnerNode>>;

		using BaseTree::BaseTree;
	};

	/*using Tree = RBTree<InnerNode, dyn_segtree_internal::InnerNodeTraits<InnerNode, Tree>,
	                    TreeOptions<TreeFlags::MULTIPLE>,
	                    dyn_segtree_internal::InnerRBTTag<Tag>, dyn_segtree_internal::Compare<InnerNode>>;
	                    */
	using KeyT = typename Node::KeyT;
	using ValueT = typename Node::ValueT;
	using AggValueT = typename Node::AggValueT;

	void insert(Node &n);

	void remove(Node &n);

	typename Node::AggValueT query(const typename Node::KeyT & x);

private:
	void apply_interval(Node & n);
	using Contour = std::pair<std::vector<InnerNode *>, std::vector<InnerNode *>>;
	Contour find_lca(InnerNode * left, InnerNode * right);

	Tree t;
};

} // namespace ygg

#include "dynamic_segment_tree.cpp"

#endif //YGG_DYNAMIC_SEGMENT_TREE_HPP
