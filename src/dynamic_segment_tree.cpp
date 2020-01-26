#ifndef YGG_DYNAMIC_SEGMENT_TREE_CPP
#define YGG_DYNAMIC_SEGMENT_TREE_CPP

#include "dynamic_segment_tree.hpp"

#include "debug.hpp"
#include "util.hpp"
#include "ygg.hpp"

#include <algorithm>
#include <iostream>
#include <tuple>

namespace ygg {

namespace dyn_segtree_internal {

template <template <class InnerNodeCRTP, class BaseKeyT> class Base,
          class OuterNode, class KeyT, class ValueT, class AggValueT,
          class Combiners, class Tag>
KeyT
InnerNode<Base, OuterNode, KeyT, ValueT, AggValueT, Combiners, Tag>::get_point()
    const noexcept
{
	return this->point;
}

template <template <class InnerNodeCRTP, class BaseKeyT> class Base,
          class OuterNode, class KeyT, class ValueT, class AggValueT,
          class Combiners, class Tag>
bool
InnerNode<Base, OuterNode, KeyT, ValueT, AggValueT, Combiners, Tag>::is_start()
    const noexcept
{
	return this->start;
}

template <template <class InnerNodeCRTP, class BaseKeyT> class Base,
          class OuterNode, class KeyT, class ValueT, class AggValueT,
          class Combiners, class Tag>
bool
InnerNode<Base, OuterNode, KeyT, ValueT, AggValueT, Combiners, Tag>::is_end()
    const noexcept
{
	return !this->start;
}

template <template <class InnerNodeCRTP, class BaseKeyT> class Base,
          class OuterNode, class KeyT, class ValueT, class AggValueT,
          class Combiners, class Tag>
bool
InnerNode<Base, OuterNode, KeyT, ValueT, AggValueT, Combiners, Tag>::is_closed()
    const noexcept
{
	return this->closed;
}

template <template <class InnerNodeCRTP, class BaseKeyT> class Base,
          class OuterNode, class KeyT, class ValueT, class AggValueT,
          class Combiners, class Tag>
const OuterNode *
InnerNode<Base, OuterNode, KeyT, ValueT, AggValueT, Combiners,
          Tag>::get_interval() const noexcept
{
	return this->container;
}

/***************************************************
 * Node traits (i.e., callbacks) for the WBTree case
 *
 * Note that we inherit all the callbacks from the RBTree,
 * thus we do not need to reimplement them.
 ***************************************************/
template <class InnerTree, class InnerNode, class Node, class NodeTraits>
template <class WBTreeBase>
void
InnerWBNodeTraits<InnerTree, InnerNode, Node, NodeTraits>::splice_out_left_knee(
    InnerNode & node, const WBTreeBase & t) noexcept
{
	// the path-sum to the right of the left knee should equal to it's
	// mirror equivalent, i.e., the path-sum to the right of the next-but-one
	// node. Just push down whatever is right
	(void)t;
	node.get_right()->agg_left += node.agg_right;
	node.get_right()->agg_right += node.agg_right;

	// Combiners may now be wrong at the right child. To fix them, we need to wipe
	// our agg_right
	node.agg_left = typename Node::AggValueT();
	node.agg_right = typename Node::AggValueT();
	InnerTree::rebuild_combiners_at(node.get_right());
}

template <class InnerTree, class InnerNode, class Node, class NodeTraits>
template <class WBTreeBase>
void
InnerWBNodeTraits<InnerTree, InnerNode, Node, NodeTraits>::
    splice_out_right_knee(InnerNode & node, const WBTreeBase & t) noexcept
{
	// the path-sum to the left of the right knee should equal to it's
	// mirror equivalent, i.e., the path-sum to the right of the previous-but-one
	// node. Just push down whatever is left
	(void)t;
	node.get_left()->agg_left += node.agg_left;
	node.get_left()->agg_right += node.agg_left;

	// Combiners may now be wrong at the right child. To fix them, we need to wipe
	// our agg_right
	node.agg_right = typename Node::AggValueT();
	node.agg_left = typename Node::AggValueT();
	InnerTree::rebuild_combiners_at(node.get_left());
}

/*template <class InnerTree, class InnerNode, class Node, class NodeTraits>
template <class WBTreeBase>
void
InnerWBNodeTraits<InnerTree, InnerNode, Node,
NodeTraits>::special_case_rl_missing(InnerNode & inserted, InnerNode &
old_parent, InnerNode & old_r, WBTreeBase & t)
{
  // This is a simulated double rotation - first right around old_r, then left
around old_parent.

}
*/

/***************************************************
 * Node traits (i.e., callbacks) for the RBTree case
 ***************************************************/
template <class InnerTree, class InnerNode, class Node, class NodeTraits>
template <class RBTreeBase>
void
InnerRBNodeTraits<InnerTree, InnerNode, Node, NodeTraits>::leaf_inserted(
    const InnerNode & node, const RBTreeBase & t) noexcept
{
	(void)node;
	(void)t;
}

template <class InnerTree, class InnerNode, class Node, class NodeTraits>
template <class RBTreeBase>
void
InnerRBNodeTraits<InnerTree, InnerNode, Node, NodeTraits>::delete_leaf(
    const InnerNode & node,
    const RBTreeBase &
        t) noexcept // TODO actually, only noexcept if the aggregation below is
{
	(void)t;

	// TODO is this really necessary everywhere we call delete_leaf?
	InnerNode * parent = node.get_parent();
	if (parent != nullptr) {
		if (parent->get_left() == &node) {
			parent->agg_left += node.agg_left;
		} else {
			parent->agg_right += node.agg_left;
		}
	}
}

template <class InnerTree, class InnerNode, class Node, class NodeTraits>
template <class RBTreeBase>
void
InnerRBNodeTraits<InnerTree, InnerNode, Node, NodeTraits>::rotated_left(
    InnerNode & node, const RBTreeBase & t) noexcept
{
	(void)t;

	InnerNode * old_right = node.get_parent();

	typename InnerNode::AggValueT old_right_agg = node.agg_right;
	node.agg_right += old_right->agg_left;

	old_right->agg_left = typename InnerNode::AggValueT();
	old_right->agg_right += old_right_agg;

	InnerTree::rebuild_combiners_at(&node);
	InnerTree::rebuild_combiners_at(old_right);
}

template <class InnerTree, class InnerNode, class Node, class NodeTraits>
template <class RBTreeBase>
void
InnerRBNodeTraits<InnerTree, InnerNode, Node, NodeTraits>::rotated_right(
    InnerNode & node, const RBTreeBase & t) noexcept
{
	(void)t;

	InnerNode * old_left = node.get_parent();

	typename InnerNode::AggValueT old_left_agg = node.agg_left;
	node.agg_left += old_left->agg_right;

	old_left->agg_right = typename InnerNode::AggValueT();
	old_left->agg_left += old_left_agg;

	InnerTree::rebuild_combiners_at(&node);
	InnerTree::rebuild_combiners_at(old_left);
}

template <class InnerTree, class InnerNode, class Node, class NodeTraits>
InnerNode *
InnerRBNodeTraits<InnerTree, InnerNode, Node, NodeTraits>::get_partner(
    const InnerNode & n) noexcept
{
	if (n.start) {
		return &(n.container->end);
	} else {
		return &(n.container->start);
	}
}

template <class InnerTree, class InnerNode, class Node, class NodeTraits>
template <class RBTreeBase>
void
InnerRBNodeTraits<InnerTree, InnerNode, Node, NodeTraits>::swapped(
    InnerNode & old_ancestor, InnerNode & old_descendant,
    RBTreeBase & t) noexcept
{
	InnerTree * it = static_cast<InnerTree *>(&t);

	/*
	std::cout << "## Fixing swap. Old ancestor: " << std::hex << &old_ancestor
	          << " / Old descendant: " << &old_descendant << std::dec << "\n";
	*/
	// Swap labels to their old places in the tree
	std::swap(old_ancestor.InnerNode::agg_left,
	          old_descendant.InnerNode::agg_left);
	std::swap(old_ancestor.InnerNode::agg_right,
	          old_descendant.InnerNode::agg_right);

	if (get_partner(old_ancestor) == &old_descendant) {
		// we are done. They have their contour nulled
		// std::cout << "### Swapped nodes are partners.\n";
		InnerTree::rebuild_combiners_at(&old_ancestor);
		InnerTree::rebuild_combiners_at(&old_descendant); // TODO is this necessary?
		return;
	}

	// Unapply the contour between where the old descendant and its partner
	InnerNode * old_descendant_partner = get_partner(old_descendant);
	const Node * old_descendant_node =
	    static_cast<const Node *>(old_descendant.container);
	auto old_descendant_val = NodeTraits::get_value(*old_descendant_node);

	if (old_descendant.InnerNode::point <
	    old_descendant_partner->InnerNode::point) {
		/*std::cout << "### Unapplying value " << old_descendant_val << " from "
		          << std::hex << &old_ancestor << " to " << old_descendant_partner
		          << std::dec << "\n";*/
		it->modify_contour(&old_ancestor, old_descendant_partner,
		                   -1 * old_descendant_val);
		/*		std::cout << "### Applying value " << old_descendant_val << " from "
		          << std::hex << &old_descendant << " to " <<
		   &old_descendant_partner
		          << std::dec << "\n";*/
		it->modify_contour(&old_descendant, old_descendant_partner,
		                   old_descendant_val);
	} else {
		// TODO
		// empty intervals break this currently
		/*		std::cout << "### 2: Unapplying value " << old_descendant_val << "
		   from "
		          << std::hex << old_descendant_partner << " to " << &old_ancestor
		          << std::dec << "\n";*/
		it->modify_contour(old_descendant_partner, &old_ancestor,
		                   -1 * old_descendant_val);
		/*		std::cout << "### 2: Applying value " << old_descendant_val << " from
		   "
		          << std::hex << old_descendant_partner << " to " << &old_descendant
		          << std::dec << "\n";*/
		it->modify_contour(old_descendant_partner, &old_descendant,
		                   old_descendant_val);
	}

	// TODO why is this necessary? Should be done by modify_contour!
	InnerTree::rebuild_combiners_at(&old_descendant);
	InnerTree::rebuild_combiners_at(&old_ancestor);
	InnerTree::rebuild_combiners_at(&old_descendant);
	InnerTree::rebuild_combiners_at(old_descendant_partner);
}

/***************************************************
 * Node traits (i.e., callbacks) for the Zip Tree case
 ***************************************************/
template <class InnerTree, class InnerNode, class AggValueT>
void
InnerZNodeTraits<InnerTree, InnerNode, AggValueT>::init_zipping(
    const InnerNode * to_be_deleted) noexcept
{
	this->left_accumulated = to_be_deleted->agg_left;
	this->right_accumulated = to_be_deleted->agg_right;
}

template <class InnerTree, class InnerNode, class AggValueT>
void
InnerZNodeTraits<InnerTree, InnerNode, AggValueT>::delete_without_zipping(
    const InnerNode * to_be_deleted) const noexcept
{
	if (to_be_deleted->get_parent() != nullptr) {
		InnerNode * parent = to_be_deleted->get_parent();
		if (parent->get_left() == to_be_deleted) {
			parent->agg_left += to_be_deleted->agg_left;
		} else {
			parent->agg_right += to_be_deleted->agg_left;
		}
	}
}

template <class InnerTree, class InnerNode, class AggValueT>
void
InnerZNodeTraits<InnerTree, InnerNode, AggValueT>::before_zip_from_left(
    InnerNode * left_head) noexcept
{
	left_head->agg_left += left_accumulated;
	this->left_accumulated += left_head->agg_right;
	left_head->agg_right = AggValueT();
}

template <class InnerTree, class InnerNode, class AggValueT>
void
InnerZNodeTraits<InnerTree, InnerNode, AggValueT>::before_zip_from_right(
    InnerNode * right_head) noexcept
{
	right_head->agg_right += right_accumulated;
	this->right_accumulated += right_head->agg_left;
	right_head->agg_left = AggValueT();
}

template <class InnerTree, class InnerNode, class AggValueT>
void
InnerZNodeTraits<InnerTree, InnerNode, AggValueT>::
    zipping_ended_left_without_tree(InnerNode * prev_left_head) const noexcept
{
	prev_left_head->agg_right = this->left_accumulated;
}

template <class InnerTree, class InnerNode, class AggValueT>
void
InnerZNodeTraits<InnerTree, InnerNode, AggValueT>::
    zipping_ended_right_without_tree(InnerNode * prev_right_head) const noexcept
{
	prev_right_head->agg_left = this->right_accumulated;
}

template <class InnerTree, class InnerNode, class AggValueT>
void
InnerZNodeTraits<InnerTree, InnerNode, AggValueT>::before_zip_tree_from_left(
    InnerNode * left_head) const noexcept
{
	left_head->agg_left += this->left_accumulated;
	left_head->agg_right += this->left_accumulated;
}

template <class InnerTree, class InnerNode, class AggValueT>
void
InnerZNodeTraits<InnerTree, InnerNode, AggValueT>::before_zip_tree_from_right(
    InnerNode * right_head) const noexcept
{
	right_head->agg_left += this->right_accumulated;
	right_head->agg_right += this->right_accumulated;
}

template <class InnerTree, class InnerNode, class AggValueT>
void
InnerZNodeTraits<InnerTree, InnerNode, AggValueT>::zipping_done(
    InnerNode * head, InnerNode * tail) const noexcept
{
	while (tail != head) {
		// TODO only iterate up if there are any combiners that must be rebuilt!
		InnerTree::rebuild_combiners_at(tail);
		tail = tail->get_parent();
	}

	// Head also needs to be rebuilt.
	InnerTree::rebuild_combiners_at(head);
}

template <class InnerTree, class InnerNode, class AggValueT>
void
InnerZNodeTraits<InnerTree, InnerNode, AggValueT>::init_unzipping(
    InnerNode * to_be_inserted) noexcept
{
	this->unzip_left_last = to_be_inserted;
	this->unzip_right_last = to_be_inserted;
	this->unzip_left_first = true;
	this->unzip_right_first = true;
	this->left_accumulated = AggValueT();
	this->right_accumulated = AggValueT();
}

template <class InnerTree, class InnerNode, class AggValueT>
void
InnerZNodeTraits<InnerTree, InnerNode, AggValueT>::unzip_to_left(
    InnerNode * n) noexcept
{
	if (unzip_left_first) {
		this->unzip_left_last->agg_left = left_accumulated;
		this->unzip_left_first = false;
	} else {
		this->unzip_left_last->agg_right = left_accumulated;
	}
	this->left_accumulated = n->agg_right;
	this->right_accumulated += n->agg_right;
	this->unzip_left_last = n;
}

template <class InnerTree, class InnerNode, class AggValueT>
void
InnerZNodeTraits<InnerTree, InnerNode, AggValueT>::unzip_to_right(
    InnerNode * n) noexcept
{
	if (unzip_right_first) {
		unzip_right_last->agg_right = right_accumulated;
		unzip_right_first = false;
	} else {
		unzip_right_last->agg_left = right_accumulated;
	}
	right_accumulated = n->agg_left;
	left_accumulated += n->agg_left;
	unzip_right_last = n;
}

template <class InnerTree, class InnerNode, class AggValueT>
void
InnerZNodeTraits<InnerTree, InnerNode, AggValueT>::unzip_done(
    InnerNode * unzip_root, InnerNode * left_spine_end,
    InnerNode * right_spine_end) noexcept
{
	/* Add final accumulated things */
	/* Both the end of the left and right spines agg_right / agg_left
	 * have been added up in left_accumulated / right_accumulated,
	 * we can set instead of add. */
	if (left_spine_end == unzip_root) {
		left_spine_end->agg_left = this->left_accumulated;
	} else {
		left_spine_end->agg_right = this->left_accumulated;
	}
	if (right_spine_end == unzip_root) {
		right_spine_end->agg_right = this->right_accumulated;
	} else {
		right_spine_end->agg_left = this->right_accumulated;
	}

	/* Rebuild left spine */
	InnerNode * n = left_spine_end;

	decltype(n->combiners) * cmb_left = nullptr;
	if (n->get_left() != nullptr) {
		cmb_left = &n->get_left()->combiners;
	}
	decltype(n->combiners) * cmb_right = nullptr;
	if (n->get_right() != nullptr) {
		cmb_right = &n->get_right()->combiners;
	}

	while (n != unzip_root) {
		n->InnerNode::combiners.rebuild(n->get_point(), cmb_left, n->agg_left,
		                                cmb_right, n->agg_right);

		n = n->get_parent();
		if (n->get_left() != nullptr) {
			cmb_left = &n->get_left()->combiners;
		} else {
			cmb_left = nullptr;
		}
		if (n->get_right() != nullptr) {
			cmb_right = &n->get_right()->combiners;
		} else {
			cmb_right = nullptr;
		}
	}

	/* Rebuild right spine */
	n = right_spine_end;
	cmb_left = nullptr;
	if (n->get_left() != nullptr) {
		cmb_left = &n->get_left()->combiners;
	}
	cmb_right = nullptr;
	if (n->get_right() != nullptr) {
		cmb_right = &n->get_right()->combiners;
	}

	while (n != unzip_root) {
		n->InnerNode::combiners.rebuild(n->get_point(), cmb_left, n->agg_left,
		                                cmb_right, n->agg_right);

		n = n->get_parent();
		if (n->get_left() != nullptr) {
			cmb_left = &n->get_left()->combiners;
		} else {
			cmb_left = nullptr;
		}
		if (n->get_right() != nullptr) {
			cmb_right = &n->get_right()->combiners;
		} else {
			cmb_right = nullptr;
		}
	}

	/* Rebuild recursively from the root up */
	InnerTree::rebuild_combiners_recursively(unzip_root);
}

/***************************************************
 * End of node traits
 ***************************************************/

template <class InnerNode>
bool
Compare<InnerNode>::operator()(const InnerNode & lhs,
                               const InnerNode & rhs) const noexcept
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
	if (lhs.get_point() != rhs.get_point()) {
#pragma GCC diagnostic pop
		return lhs.get_point() < rhs.get_point();
	} else {
		/*
		 * At the same point, the order is: open ends, closed starts, closed ends,
		 * open starts
		 */
		int_fast8_t lhs_priority;
		if (lhs.is_closed()) {
			if (lhs.is_start()) {
				lhs_priority = -1;
			} else {
				lhs_priority = 1;
			}
		} else {
			if (lhs.is_start()) {
				lhs_priority = 2;
			} else {
				lhs_priority = -2;
			}
		}

		int_fast8_t rhs_priority;
		if (rhs.is_closed()) {
			if (rhs.is_start()) {
				rhs_priority = -1;
			} else {
				rhs_priority = 1;
			}
		} else {
			if (rhs.is_start()) {
				rhs_priority = 2;
			} else {
				rhs_priority = -2;
			}
		}

		return lhs_priority < rhs_priority;
	}
}

template <class InnerNode>
bool
Compare<InnerNode>::operator()(const typename InnerNode::KeyT & lhs,
                               const InnerNode & rhs) const noexcept
{
	//    std::cout << "A Comparing points: " << lhs << " vs " <<
	//    rhs.get_point() << "\n";
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
	if (lhs != rhs.get_point()) {
		return lhs < rhs.get_point();
	}
#pragma GCC diagnostic pop

	// only open starts are strictly larger than the key
	return (rhs.is_start() && !rhs.is_closed());
}

template <class InnerNode>
bool
Compare<InnerNode>::operator()(const InnerNode & lhs,
                               const typename InnerNode::KeyT & rhs) const
    noexcept
{
	//    std::cout << "B Comparing points: " << lhs.get_point() << " vs " <<
	//    rhs << "\n";
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
	if (lhs.get_point() != rhs) {
		return lhs.get_point() < rhs;
	}
#pragma GCC diagnostic pop

	// only open ends must strictly go before the key
	return (!lhs.is_start() && !lhs.is_closed());
}

template <class InnerNode>
bool
Compare<InnerNode>::operator()(const PointDescription & lhs,
                               const InnerNode & rhs) const noexcept
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
	if (lhs.first != rhs.get_point()) {
		return lhs.first < rhs.get_point();
	}
#pragma GCC diagnostic pop

	if (lhs.second > 0) {
		// the query is left-open, i.e., it should not to before anything
		return false;
	} else if (lhs.second < 0) {
		// the query is right-open. It should go before everything but open end
		// nodes
		return rhs.is_start() || rhs.is_closed();
	} else {
		// The query is closed.
		// only open starts are strictly larger than the key
		return (rhs.is_start() && !rhs.is_closed());
	}
}

template <class InnerNode>
bool
Compare<InnerNode>::operator()(const InnerNode & lhs,
                               const PointDescription & rhs) const noexcept
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
	if (lhs.get_point() != rhs.first) {
		return lhs.get_point() < rhs.first;
	}
#pragma GCC diagnostic pop

	if (rhs.second > 0) {
		// the query is left-open, i.e., everything but left-open is before it
		return !(lhs.is_start() && !lhs.is_closed());
	} else if (rhs.second > 0) {
		// the query is right-open, i.e., nothing must ever strictly go before it
		return false;
	} else {
		// the query is closed
		// only open ends must strictly go before the key
		return (!lhs.is_start() && !lhs.is_closed());
	}
}

/*
 * Debugging methods
 */
template <class InnerNode, class... Combiners>
std::string
ASCIIInnerNodeNameGetter<InnerNode, Combiners...>::get_name(
    InnerNode * node) const
{
	std::vector<std::string> combiner_txts{
	    Combiners::get_name() + std::string(": ") +
	    node->combiners.template get_combiner<Combiners>().get_dbg_value() +
	    std::string(" ")...};

	std::ostringstream res;
	res << std::string("{") << std::to_string(node->get_point())
	    << std::string("}") << std::string("[");

	if (node->is_start()) {
		res << "_";
	}
	res << std::to_string(node->get_interval()->start.get_point());
	if (node->is_start()) {
		res << "_";
	}

	res << std::string(", ");
	if (node->is_end()) {
		res << "_";
	}
	res << std::to_string(node->get_interval()->end.get_point());
	if (node->is_end()) {
		res << "_";
	}

	res << std::string(") ") << std::string("@") << std::hex << node << std::dec
	    << std::string("  ╭:") << std::to_string(node->agg_left)
	    << std::string("  ╮:") << std::to_string(node->agg_right)
	    << std::string("  {");

	bool first = true;
	for (auto & cmb_str : combiner_txts) {
		if (!first) {
			res << ", ";
		}
		first = false;
		res << cmb_str;
	}

	res << std::string("}");

	return res.str();
}

template <class InnerNode, class... Combiners>
std::string
DOTInnerNodeNameGetter<InnerNode, Combiners...>::get_name(
    InnerNode * node) const
{
	std::stringstream name;

	std::string combiner_str{
	    Combiners::get_name() + std::string(": ") +
	    std::to_string(node->combiners.template get<Combiners>()) +
	    std::string(" ")...};

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

template <class InnerNode>
std::string
DOTInnerEdgeNameGetter<InnerNode>::get_name(InnerNode * node, bool left) const
{
	if (left) {
		return std::to_string(node->agg_left);
	} else {
		return std::to_string(node->agg_right);
	}
}

} // namespace dyn_segtree_internal

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
void
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::dbg_print_inner_tree() const
{
	TreePrinter tp(this->t.get_root(), NodeNameGetter());
	tp.print();
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
std::stringstream &
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::dbg_get_dot() const
{
	TreeDotExporter tdot(
	    this->t.get_root(), DotNameGetter(),
	    dyn_segtree_internal::DOTInnerEdgeNameGetter<InnerNode>());
	return tdot.get();
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
void
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::insert(Node & n)
{
	// TODO why are we doing this every time? Should be done once in the
	// constructor!
	n.NB::start.point = NodeTraits::get_lower(n);
	n.NB::start.closed = NodeTraits::is_lower_closed(n);
	n.NB::start.agg_left = AggValueT();
	n.NB::start.agg_right = AggValueT();

	n.NB::start.start = true;
	n.NB::start.container = static_cast<NB *>(&n);

	if constexpr (utilities::is_specialization<TreeSelector, UseZipTree>{} &&
	              InnerOptions::ztree_use_hash &&
	              InnerOptions::ztree_store_rank) {
		// We need to update the ranks
		n.NB::start.update_rank();
		n.NB::end.update_rank();
	}

	this->t.insert(n.NB::start);
	n.NB::end.point = NodeTraits::get_upper(n);
	n.NB::end.closed = NodeTraits::is_upper_closed(n);
	n.NB::end.agg_left = AggValueT();
	n.NB::end.agg_right = AggValueT();

	n.NB::end.start = false;
	n.NB::end.container = static_cast<NB *>(&n);

	this->t.insert(n.NB::end);

	this->apply_interval(n);
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::DynamicSegmentTree(MyClass && other)
    : t(std::move(other.t))
{}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::DynamicSegmentTree()
    : t()
{}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
void
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::remove(Node & n)
{
	//	std::cout << "############### Unapplying interval\n";

	this->unapply_interval(n);

	// this->dbg_print_inner_tree();
	// std::cout << "############### Removing at " << n.NB::start.get_point()
	// << "\n";

	this->t.remove(n.NB::start);

	// this->dbg_print_inner_tree();
	// std::cout << "############### Removing at " << n.NB::end.get_point() <<
	// "\n";
	this->t.remove(n.NB::end);

	// this->dbg_print_inner_tree();
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
void
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::InnerTree::build_lca(InnerNode * left,
                                              InnerNode * right) const
{
	this->generation++;

	// TODO buffer these
	this->contour_left_path.clear();
	this->contour_right_path.clear();

	InnerNode * lca = nullptr;

	this->contour_left_path.push_back(left);
	this->contour_right_path.push_back(right);

	if (left == right) {
		return;
	}

	left->lca_tag = this->generation;
	right->lca_tag = this->generation;

	bool progress_left = true;
	while (true) {
		if (progress_left) {
			left = left->get_parent();

			if (__builtin_expect((left != nullptr), 1)) {
				this->contour_left_path.push_back(left);

				if (__builtin_expect((left->lca_tag == this->generation), 0)) {
					// Already visited by the right search
					lca = left;
					break;
				}

				left->lca_tag = this->generation;
			}
		} else {
			right = right->get_parent();

			if (__builtin_expect((right != nullptr), 1)) {
				this->contour_right_path.push_back(right);

				if (__builtin_expect((right->lca_tag == this->generation), 0)) {
					// Already visited by the left search
					lca = right;
					break;
				}

				right->lca_tag = this->generation;
			}
		}

		if ((left != nullptr) && (right != nullptr)) {
			progress_left = !progress_left;
		} else if (left == nullptr) {
			progress_left = false;
		} else {
			progress_left = true;
		}
	}

	while (this->contour_left_path.back() != lca) {
		this->contour_left_path.pop_back();
	}
	while (this->contour_right_path.back() != lca) {
		this->contour_right_path.pop_back();
	}
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
void
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::apply_interval(Node & n)
{
	this->t.modify_contour(&n.NB::start, &n.NB::end, NodeTraits::get_value(n));
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
void
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::unapply_interval(Node & n)
{
	this->t.modify_contour(&n.NB::start, &n.NB::end,
	                       -1 * NodeTraits::get_value(n));
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
void
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::InnerTree::modify_contour(InnerNode * left,
                                                   InnerNode * right,
                                                   ValueT val)
{
	this->build_lca(left, right);

	// left contour
	bool last_changed_left = false;
	for (size_t i = 0; i < this->contour_left_path.size() - 1; ++i) {
		InnerNode * cur = this->contour_left_path[i];
		if ((i == 0) || (cur->get_right() != this->contour_left_path[i - 1])) {
			cur->InnerNode::agg_right += val;
		}
		last_changed_left = rebuild_combiners_at(cur);
	}

	// right contour
	bool last_changed_right = false;
	for (size_t i = 0; i < this->contour_right_path.size() - 1; ++i) {
		InnerNode * cur = this->contour_right_path[i];
		if ((i == 0) || (cur->get_left() != this->contour_right_path[i - 1])) {
			cur->InnerNode::agg_left += val;
		}
		last_changed_right = rebuild_combiners_at(cur);
	}

	if (last_changed_left || last_changed_right) {
		InnerNode * lca = (!this->contour_left_path.empty())
		                      ? this->contour_left_path.back()
		                      : this->contour_right_path.back();
		rebuild_combiners_recursively(lca);
	}
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
bool
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::empty() const
{
	return this->t.empty();
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
void
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::clear()
{
	return this->t.clear();
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
void
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::dbg_verify_start_end() const
{
	for (auto & n : this->t) {
		auto outer = n.get_interval();
		bool should_be_start = &(outer->start) == &n;
		debug::yggassert(should_be_start == n.is_start());
		decltype(&(outer->end)) partner;
		if (should_be_start) {
			partner = &outer->end;
			debug::yggassert(partner->get_point() > n.get_point());
		} else {
			partner = &outer->start;
			debug::yggassert(partner->get_point() < n.get_point());
		}
	}
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
void
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::dbg_verify_all_points() const
{
	using Point = std::pair<typename Node::KeyT, typename Node::AggValueT>;

	std::set<const Node *> nodes;
	std::vector<const Node *> starts;
	std::vector<const Node *> ends;
	std::vector<Point> points;

	for (auto & n : this->t) {
		const Node * node = static_cast<const Node *>(n.get_interval());
		if (nodes.find(node) == nodes.end()) {
			nodes.insert(node);
			starts.push_back(node);
			ends.push_back(node);
		}
	}

	if (nodes.empty()) {
		return;
	}

	std::sort(starts.begin(), starts.end(),
	          [](const Node * lhs, const Node * rhs) {
		          return lhs->start.get_point() > rhs->start.get_point();
	          });
	std::sort(ends.begin(), ends.end(), [](const Node * lhs, const Node * rhs) {
		return lhs->end.get_point() > rhs->end.get_point();
	});

	KeyT last_point = starts.back()->start.get_point();
	AggValueT last_val = 0; // TODO only works for numeric types!
	while ((!starts.empty()) || (!ends.empty())) {
		KeyT new_point;
		AggValueT delta;
		if ((!ends.empty()) &&
		    ((starts.empty()) ||
		     (ends.back()->end.get_point() <= starts.back()->start.get_point()))) {
			new_point = ends.back()->end.get_point();
			delta = -1 * NodeTraits::get_value(*ends.back());
			ends.pop_back();
		} else {
			new_point = starts.back()->start.get_point();
			delta = NodeTraits::get_value(*starts.back());
			starts.pop_back();
		}

		if ((new_point > last_point) &&
		    (new_point < std::numeric_limits<KeyT>::max() / 2)) {
			points.emplace_back(last_point, last_val);
			KeyT point_between = (last_point + new_point) / 2;
			if ((point_between != last_point) && (point_between != new_point)) {
				points.emplace_back(point_between, last_val);
			}
		}

		last_val += delta;
		last_point = new_point;
	}

	AggValueT epsilon = 0;
	if (!std::numeric_limits<AggValueT>::is_integer) { // TODO should be if
		                                                 // constexpr
		epsilon = 0.000001;
	}

	for (const auto & point : points) {
		auto result = this->query(point.first);
#ifdef NDEBUG
		(void)result; // Avoid warnings
		(void)epsilon;
#endif
		assert(result <= point.second + epsilon);
		assert(result >= point.second - epsilon);
	}
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
void
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::dbg_verify() const
{
	this->dbg_verify_start_end();
	this->dbg_verify_all_points();
	this->t.dbg_verify();
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
typename Node::AggValueT
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::query(const typename Node::KeyT & x) const noexcept
{
	InnerNode * cur = this->t.get_root();
	AggValueT agg = AggValueT();

	dyn_segtree_internal::Compare<InnerNode> cmp;

	while (cur != nullptr) {
		if (cmp(x, *cur)) {
			agg += cur->agg_left;
			cur = cur->get_left();
		} else {
			agg += cur->agg_right;
			cur = cur->get_right();
		}
	}

	return agg;
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
template <class Combiner>
Combiner
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::get_combiner(const typename Node::KeyT & lower,
                                      const typename Node::KeyT & upper,
                                      bool lower_closed,
                                      bool upper_closed) const
{
	dyn_segtree_internal::Compare<InnerNode> cmp;

	decltype(this->t.lower_bound(lower)) lower_node_it;
	if (lower_closed) {
		lower_node_it = this->t.lower_bound(lower);
	} else {
		lower_node_it = this->t.lower_bound(
		    std::pair<const typename Node::KeyT &, const int_fast8_t>{lower, +1});
	}
	InnerNode * lower_node;

	if (lower_node_it == this->t.end()) {
		auto lower_node_rit = this->t.rbegin();
		lower_node = const_cast<InnerNode *>(&*lower_node_rit);
	} else {
		if (cmp(lower, *lower_node_it)) {
			// we must go one further back
			if (lower_node_it != this->t.begin()) {
				lower_node_it--;
			}
		}
		lower_node = const_cast<InnerNode *>(&*lower_node_it);
	}

	decltype(this->t.upper_bound(upper)) upper_node_it;
	if (upper_closed) {
		upper_node_it = this->t.upper_bound(upper);
	} else {
		upper_node_it = this->t.upper_bound(
		    std::pair<const typename Node::KeyT &, const int_fast8_t>{upper, -1});
	}

	InnerNode * upper_node;
	if (upper_node_it == this->t.end()) {
		auto upper_node_rit = this->t.rbegin();
		upper_node = const_cast<InnerNode *>(&*upper_node_rit);
	} else {
		upper_node = const_cast<InnerNode *>(&*upper_node_it);
	}

	this->t.build_lca(lower_node, upper_node);

	// TODO inefficient: We don't need to build all the combiners!
	Combiners dummy_cp;
	Combiners cp;

	decltype(InnerNode().get_point()) topmost_point;

	if (!this->t.contour_left_path.empty()) {
		for (size_t i = 0; i < this->t.contour_left_path.size(); ++i) {
			InnerNode * cur = this->t.contour_left_path[i];
			InnerNode * right_child = cur->get_right();

			// Factor in the edge we just traversed up
			if (i > 0) {
				// TODO inefficient!
				if (right_child == this->t.contour_left_path[i - 1]) {
					// we traversed the right edge
					cp.traverse_right_edge_up(cur->get_point(), cur->agg_right);
				} else {
					// we traversed the left edge
					cp.traverse_left_edge_up(cur->get_point(), cur->agg_left);
				}
			}

			// Combine with descending across the contour, if we traversed a left edge
			if ((i != this->t.contour_left_path.size() - 1) &&
			    ((i == 0) || (right_child != this->t.contour_left_path[i - 1]))) {
				if (right_child != nullptr) {
					cp.collect_right(cur->get_point(), &right_child->combiners,
					                 cur->agg_right);
				} else {
					cp.collect_right(cur->get_point(), nullptr, cur->agg_right);
				}
			}
		}

		topmost_point = this->t.contour_left_path.back()->get_point();
	}

	// TODO is this necessary?
	Combiners left_cp = cp;
	cp = Combiners();

	// TODO can this ever be empty?
	// Traverse the last left-edge into the root
	if (!this->t.contour_right_path.empty()) {
		for (size_t i = 0; i < this->t.contour_right_path.size(); ++i) {
			InnerNode * cur = this->t.contour_right_path[i];
			InnerNode * left_child = cur->get_left();

			// Factor in the edge we just traversed up
			if (i > 0) {
				if (left_child == this->t.contour_right_path[i - 1]) {
					// we traversed the left edge
					cp.traverse_left_edge_up(cur->get_point(), cur->agg_left);
				} else {
					// we traversed the right edge
					cp.traverse_right_edge_up(cur->get_point(), cur->agg_right);
				}
			}

			// Combine with descending across the contour, if we traversed a right
			// edge
			if ((i != this->t.contour_right_path.size() - 1) &&
			    ((i == 0) || (left_child != this->t.contour_right_path[i - 1]))) {
				if (left_child != nullptr) {
					cp.collect_left(cur->get_point(), &left_child->combiners,
					                cur->agg_left);
				} else {
					cp.collect_left(cur->get_point(), nullptr, cur->agg_left);
				}
			}
		}

		topmost_point = this->t.contour_right_path.back()->get_point();
	}

	// Combine right and left contour
	cp.collect_left(topmost_point, &left_cp, typename Node::AggValueT());

	/*
	 * Step 3: Take the combined value and aggregate into it everything on the way
	 * up to the root.
	 */
	InnerNode * cur = this->t.contour_right_path.back();
	while (cur != this->t.get_root()) {
		InnerNode * old = cur;
		cur = cur->get_parent();
		if (cur->get_left() == old) {
			cp.traverse_left_edge_up(cur->get_point(), cur->agg_left);
		} else {
			cp.traverse_right_edge_up(cur->get_point(), cur->agg_right);
		}
	}

	return cp.template get_combiner<Combiner>();
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
template <class Combiner>
typename Combiner::ValueT
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::get_combined(const typename Node::KeyT & lower,
                                      const typename Node::KeyT & upper,
                                      bool lower_closed,
                                      bool upper_closed) const
{
	return this->get_combiner<Combiner>(lower, upper, lower_closed, upper_closed)
	    .get();
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
bool
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::InnerTree::rebuild_combiners_at(InnerNode * n)
{
	Combiners * cmb_left = nullptr;
	if (n->get_left() != nullptr) {
		cmb_left = &n->get_left()->combiners;
	}
	Combiners * cmb_right = nullptr;
	if (n->get_right() != nullptr) {
		cmb_right = &n->get_right()->combiners;
	}

	return n->combiners.rebuild(n->get_point(), cmb_left, n->agg_left, cmb_right,
	                            n->agg_right);
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
void
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::InnerTree::rebuild_combiners_recursively(InnerNode * n)
{
	Combiners * cmb_left = nullptr;
	if (n->get_left() != nullptr) {
		cmb_left = &n->get_left()->combiners;
	}
	Combiners * cmb_right = nullptr;
	if (n->get_right() != nullptr) {
		cmb_right = &n->get_right()->combiners;
	}

	while (n->InnerNode::combiners.rebuild(n->get_point(), cmb_left, n->agg_left,
	                                       cmb_right, n->agg_right)) {
		n = n->get_parent();

		if (n != nullptr) {
			if (n->get_left() != nullptr) {
				cmb_left = &n->get_left()->combiners;
			} else {
				cmb_left = nullptr;
			}
			if (n->get_right() != nullptr) {
				cmb_right = &n->get_right()->combiners;
			} else {
				cmb_right = nullptr;
			}
		} else {
			break;
		}
	}
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
typename DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                            Tag>::template const_iterator<false>
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::cbegin() const
{
	return this->t.cbegin();
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
typename DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                            Tag>::template const_iterator<false>
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::cend() const
{
	return this->t.cend();
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
typename DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                            Tag>::template const_iterator<false>
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::begin() const
{
	return this->t.begin();
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
typename DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                            Tag>::template iterator<false>
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::begin()
{
	return this->t.begin();
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
typename DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                            Tag>::template iterator<false>
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::end()
{
	return this->t.end();
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
typename DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                            Tag>::template const_iterator<false>
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::end() const
{
	return this->t.end();
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
typename DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                            Tag>::template const_iterator<true>
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::crbegin() const
{
	return this->t.crbegin();
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
typename DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                            Tag>::template const_iterator<true>
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::crend() const
{
	return this->t.crend();
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
typename DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                            Tag>::template const_iterator<true>
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::rbegin() const
{
	return this->t.rbegin();
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
typename DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                            Tag>::template iterator<true>
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::rbegin()
{
	return this->t.rbegin();
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
typename DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                            Tag>::template iterator<true>
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::rend()
{
	return this->t.rend();
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
typename DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                            Tag>::template const_iterator<true>
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::rend() const
{
	return this->t.rend();
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
typename DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                            Tag>::template const_iterator<false>
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::lower_bound_event(const KeyT & key) const
{
	return this->t.lower_bound(key);
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
typename DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                            Tag>::template iterator<false>
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::lower_bound_event(const KeyT & key)
{
	return this->t.lower_bound(key);
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
typename DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                            Tag>::template const_iterator<false>
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::upper_bound_event(const KeyT & key) const
{
	return this->t.upper_bound(key);
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
typename DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                            Tag>::template iterator<false>
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::upper_bound_event(const KeyT & key)
{
	return this->t.upper_bound(key);
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
template <class Combiner>
Combiner
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::get_combiner() const
{
	if (this->t.get_root() == nullptr) {
		return Combiner();
	}

	return this->t.get_root()->combiners.template get_combiner<Combiner>();
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
template <class Combiner>
typename Combiner::ValueT
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::get_combined() const
{
	return this->get_combiner<Combiner>().get();
}

template <class Node, class NodeTraits, class Combiners, class Options,
          class TreeSelector, class Tag>
template <class Combiner>
void
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, TreeSelector,
                   Tag>::dbg_verify_max_combiner() const
{
	typename Node::AggValueT max_val{};
	typename Node::AggValueT delta{};
	// TODO FIXME this breaks for non-numeric types
	if (!std::numeric_limits<typename Node::AggValueT>::is_integer) {
		delta = 0.000001;
	} else {
		(void)delta;
	}

	for (auto & node : this->t) {
		if (node.get_left() != nullptr) {
			auto left_child = node.get_left();
			max_val =
			    std::max(max_val, left_child->combiners.template get<Combiner>() +
			                          node.agg_left);
			assert(node.combiners.template get<Combiner>() + delta >=
			       left_child->combiners.template get<Combiner>() + node.agg_left);
			(void)left_child; // GCC complains about the variable not being used
		} else {
			max_val = std::max(max_val, node.agg_left);
			assert(node.combiners.template get<Combiner>() + delta >= node.agg_left);
		}

		if (node.get_right() != nullptr) {
			auto right_child = node.get_right();
			max_val =
			    std::max(max_val, right_child->combiners.template get<Combiner>() +
			                          node.agg_right);
			assert(node.combiners.template get<Combiner>() + delta >=
			       right_child->combiners.template get<Combiner>() + node.agg_right);
			(void)right_child;
		} else {
			assert(node.combiners.template get<Combiner>() + delta >= node.agg_right);
			max_val = std::max(max_val, node.agg_right);
		}

		assert(node.combiners.template get<Combiner>() - delta <= max_val);
	}
}

/********************************************************
 *
 * RangedMaxCombiner
 *
 ********************************************************
 */

template <class KeyT, class ValueT>
RangedMaxCombiner<KeyT, ValueT>::RangedMaxCombiner()
    : val(ValueT()), left_border(KeyT()), left_border_valid(false),
      right_border(KeyT()), right_border_valid(false)
{}

template <class KeyT, class ValueT>
bool
RangedMaxCombiner<KeyT, ValueT>::traverse_left_edge_up(KeyT new_point,
                                                       ValueT edge_val)
{
	this->val += edge_val;

	if (this->right_border_valid) {
		// TODO is this even necessary?
		this->right_border = std::min(new_point, this->right_border);
	} else {
		this->right_border = new_point;
		this->right_border_valid = true;
	}

	return false;
}

template <class KeyT, class ValueT>
bool
RangedMaxCombiner<KeyT, ValueT>::traverse_right_edge_up(KeyT new_point,
                                                        ValueT edge_val)
{
	this->val += edge_val;

	if (this->left_border_valid) {
		this->left_border = std::max(new_point, this->left_border);
	} else {
		this->left_border = new_point;
		this->left_border_valid = true;
	}

	return false;
}

template <class KeyT, class ValueT>
bool
RangedMaxCombiner<KeyT, ValueT>::collect_left(
    const KeyT my_point, const MyType * left_child_combiner,
    const ValueT edge_val)
{
	const auto new_candidate_value = child_value(left_child_combiner) + edge_val;

	// In case that neither border is valid, this object has not been initialized
	// with a Value yet: We therefore take the offered value and set our first
	// border!
	if ((new_candidate_value > this->val) ||
	    (!this->right_border_valid && !this->left_border_valid)) {
		this->val = new_candidate_value;

		if (left_child_combiner != nullptr) {
			this->left_border = left_child_combiner->left_border;
			this->left_border_valid = left_child_combiner->left_border_valid;

			if (left_child_combiner->right_border_valid) {
				this->right_border =
				    std::min(my_point, left_child_combiner->right_border);
			} else {
				this->right_border = my_point;
			}
		} else {
			this->left_border_valid = false;

			this->right_border = my_point;
		}
		this->right_border_valid = true;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
	} else if (new_candidate_value == this->val) {
#pragma GCC diagnostic pop
		// In case the values are equal, we need to check for merging of our ranges!

		if (left_child_combiner != nullptr) {
			if ((!this->left_border_valid ||
			     (this->left_border <=
			      my_point)) && // Compatible to merge from our point
			    (!left_child_combiner->right_border_valid ||
			     (left_child_combiner->right_border >= my_point)))
			// Compatible to merge from left-childs point
			{
				// merging!
				this->left_border = left_child_combiner->left_border;
				this->left_border_valid = left_child_combiner->left_border_valid;
			} else {
				// Two disjunct intervals with the same max value. Prefer the left one!
				this->left_border = left_child_combiner->left_border;
				this->left_border_valid = left_child_combiner->left_border_valid;
				this->right_border = left_child_combiner->right_border;
				this->right_border_valid = true;
			}
		} else {
			// We have an unlimited interval of the same value to our left
			if (my_point >= this->left_border) {
				// And it's compatible with our border!
				this->left_border_valid = false;
			} else {
				// Take the unlimited value as new left-most maximum interval
				this->right_border = my_point;
				this->right_border_valid = true;
				this->left_border_valid = false;
			}
		}
	}

	return false;
}

template <class KeyT, class ValueT>
bool
RangedMaxCombiner<KeyT, ValueT>::collect_right(
    const KeyT my_point, const MyType * right_child_combiner,
    const ValueT edge_val)
{
	const auto new_candidate_value = child_value(right_child_combiner) + edge_val;

	if ((new_candidate_value > this->val) ||
	    (!this->right_border_valid && !this->left_border_valid)) {
		this->val = new_candidate_value;

		if (right_child_combiner != nullptr) {
			this->right_border_valid = right_child_combiner->right_border_valid;
			this->right_border = right_child_combiner->right_border;

			if (right_child_combiner->left_border_valid) {
				this->left_border =
				    std::max(my_point, right_child_combiner->left_border);
			} else {
				this->left_border = my_point;
			}
		} else {
			this->right_border_valid = false;

			this->left_border = my_point;
		}

		this->left_border_valid = true;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
	} else if (new_candidate_value == this->val) {
#pragma GCC diagnostic pop
		// In case the values are equal, we need to check for merging of our ranges!

		if (right_child_combiner != nullptr) {
			if ((!this->right_border_valid ||
			     (this->right_border >=
			      my_point)) && // Compatible to merge from our point
			    (!right_child_combiner->left_border_valid ||
			     (right_child_combiner->left_border <= my_point)))
			// Compatible to merge from right-childs point
			{
				// merging!
				this->right_border = right_child_combiner->right_border;
				this->right_border_valid = right_child_combiner->right_border_valid;
			} else {
				// Two disjunct intervals with the same max value. Prefer the left one
				// (i.e., ours). Nothing to do!
			}
		} else {
			// We have a unlimited interval of the same value to our right
			if (my_point <= this->right_border) {
				// And it's compatible with our border!
				this->right_border_valid = false;
			}
		}
	}

	return false;
}

template <class KeyT, class ValueT>
ValueT
RangedMaxCombiner<KeyT, ValueT>::get() const noexcept
{
	return this->val;
}

template <class KeyT, class ValueT>
bool
RangedMaxCombiner<KeyT, ValueT>::rebuild(KeyT my_point,
                                         const MyType * left_child_combiner,
                                         ValueT left_edge_val,
                                         const MyType * right_child_combiner,
                                         ValueT right_edge_val)
{
	auto old_val = this->val;

	auto left_val = child_value(left_child_combiner) + left_edge_val;
	auto right_val = child_value(right_child_combiner) + right_edge_val;

	if (left_val > right_val) {
		this->val = left_val;

		if (left_child_combiner != nullptr) {
			this->left_border = left_child_combiner->left_border;
			this->left_border_valid = left_child_combiner->left_border_valid;
			if (left_child_combiner->right_border_valid) {
				this->right_border =
				    std::min(my_point, left_child_combiner->right_border);
			} else {
				this->right_border = my_point;
			}
			this->right_border_valid = true;
		} else {
			this->left_border_valid = false;
			this->right_border = my_point;
			this->right_border_valid = true;
		}
	} else if (right_val > left_val) {
		this->val = right_val;

		if (right_child_combiner != nullptr) {
			this->right_border = right_child_combiner->right_border;
			this->right_border_valid = right_child_combiner->right_border_valid;
			if (right_child_combiner->left_border_valid) {
				this->left_border =
				    std::max(my_point, right_child_combiner->left_border);
			} else {
				this->left_border = my_point;
			}
			this->left_border_valid = true;
		} else {
			this->right_border_valid = false;
			this->left_border = my_point;
			this->left_border_valid = true;
		}
	} else {
		// Both are equal!
		this->val = left_val;

		/* For merging, it must hold that the right border of the left interval is
		 * greater or equal to our point (or unlimited), and the left border of the
		 * right interval must be less or equal to our point (or unlimited).
		 */

		bool merge_candidate_left =
		    ((left_child_combiner == nullptr) ||
		     (!left_child_combiner->right_border_valid) ||
		     (left_child_combiner->right_border >= my_point));
		bool merge_candidate_right =
		    ((right_child_combiner == nullptr) ||
		     (!right_child_combiner->left_border_valid) ||
		     (right_child_combiner->left_border <= my_point));

		if (merge_candidate_left && merge_candidate_right) {
			// Yes, we're merging!

			if (right_child_combiner != nullptr) {
				this->right_border = right_child_combiner->right_border;
				this->right_border_valid = right_child_combiner->right_border_valid;
			} else {
				this->right_border_valid = false;
			}

			if (left_child_combiner != nullptr) {
				this->left_border = left_child_combiner->left_border;
				this->left_border_valid = left_child_combiner->left_border_valid;
			} else {
				this->left_border_valid = false;
			}
		} else {
			// No merging - prefer left
			if (left_child_combiner != nullptr) {
				this->left_border = left_child_combiner->left_border;
				this->left_border_valid = left_child_combiner->left_border_valid;

				if (left_child_combiner->right_border_valid) {
					this->right_border =
					    std::min(my_point, left_child_combiner->right_border);
				} else {
					this->right_border = my_point;
				}
				this->right_border_valid = true;
			} else {
				this->left_border_valid = false;
				this->right_border = my_point;
				this->right_border_valid = true;
			}
		}
	}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
	return old_val != this->val;
#pragma GCC diagnostic pop
}

template <class KeyT, class ValueT>
ValueT
RangedMaxCombiner<KeyT, ValueT>::child_value(const MyType * child) const
    noexcept
{
	if (child == nullptr) {
		return ValueT();
	}

	return child->get();
}

template <class KeyT, class ValueT>
KeyT
RangedMaxCombiner<KeyT, ValueT>::get_left_border() const noexcept
{
	return this->left_border;
}

template <class KeyT, class ValueT>
KeyT
RangedMaxCombiner<KeyT, ValueT>::get_right_border() const noexcept
{
	return this->right_border;
}

template <class KeyT, class ValueT>
bool
RangedMaxCombiner<KeyT, ValueT>::is_left_border_valid() const noexcept
{
	return this->left_border_valid;
}

template <class KeyT, class ValueT>
bool
RangedMaxCombiner<KeyT, ValueT>::is_right_border_valid() const noexcept
{
	return this->right_border_valid;
}

/********************************************************
 *
 * MaxCombiner
 *
 ********************************************************
 */

template <class KeyT, class ValueT>
bool
MaxCombiner<KeyT, ValueT>::traverse_left_edge_up(const KeyT new_point,
                                                 const ValueT edge_val)
{
	(void)new_point;
	this->val += edge_val;
	return false;
}

template <class KeyT, class ValueT>
bool
MaxCombiner<KeyT, ValueT>::traverse_right_edge_up(const KeyT new_point,
                                                  const ValueT edge_val)
{
	(void)new_point;
	this->val += edge_val;
	return false;
}

template <class KeyT, class ValueT>
bool
MaxCombiner<KeyT, ValueT>::collect_left(const KeyT my_point,
                                        const MyType * left_child_combiner,
                                        const ValueT edge_val)
{
	(void)my_point;
	this->val = std::max(this->val, child_value(left_child_combiner) + edge_val);
	return false;
}

template <class KeyT, class ValueT>
bool
MaxCombiner<KeyT, ValueT>::collect_right(KeyT my_point,
                                         const MyType * right_child_combiner,
                                         ValueT edge_val)
{
	(void)my_point;
	this->val = std::max(this->val, child_value(right_child_combiner) + edge_val);
	return false;
}

template <class KeyT, class ValueT>
ValueT
MaxCombiner<KeyT, ValueT>::get() const noexcept
{
	return this->val;
}

template <class KeyT, class ValueT>
bool
MaxCombiner<KeyT, ValueT>::rebuild(KeyT my_point,
                                   const MyType * left_child_combiner,
                                   ValueT left_edge_val,
                                   const MyType * right_child_combiner,
                                   ValueT right_edge_val)
{
	(void)my_point;
	auto old_val = this->val;

	this->val = std::max(child_value(left_child_combiner) + left_edge_val,
	                     child_value(right_child_combiner) + right_edge_val);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
	return old_val != val;
#pragma GCC diagnostic pop
}

template <class KeyT, class ValueT>
ValueT
MaxCombiner<KeyT, ValueT>::child_value(const MaxCombiner::MyType * child) const
    noexcept
{
	if (child == nullptr) {
		return ValueT();
	}

	return child->get();
}

/********************************************************
 *
 * CombinerPack
 *
 ********************************************************
 */

template <class KeyT, class AggValueT, class... Combiners>
bool
CombinerPack<KeyT, AggValueT, Combiners...>::rebuild(KeyT my_point,
                                                     const MyType * left_child,
                                                     AggValueT left_edge_val,
                                                     const MyType * right_child,
                                                     AggValueT right_edge_val)
{
	if constexpr (sizeof...(Combiners) == 0) {
		(void)my_point;
		(void)left_child;
		(void)left_edge_val;
		(void)right_child;
		(void)right_edge_val;
		return false;
	} else {
		return (std::get<Combiners>(this->data)
		            .rebuild(my_point, child_combiner<Combiners>(left_child),
		                     left_edge_val, child_combiner<Combiners>(right_child),
		                     right_edge_val) |
		        ...);
	}
} // namespace ygg

template <class KeyT, class AggValueT, class... Combiners>
bool
CombinerPack<KeyT, AggValueT, Combiners...>::collect_left(
    KeyT my_point, const MyType * left_child_combiner, AggValueT edge_val)
{
	// This is a fold expression with a comma operator!
	(std::get<Combiners>(this->data)
	     .collect_left(my_point, child_combiner<Combiners>(left_child_combiner),
	                   edge_val),
	 ...);
	return false;
}

template <class KeyT, class AggValueT, class... Combiners>
bool
CombinerPack<KeyT, AggValueT, Combiners...>::collect_right(
    KeyT my_point, const MyType * right_child_combiner, AggValueT edge_val)
{
	// This is a fold expression with a comma operator!
	(std::get<Combiners>(this->data)
	     .collect_right(my_point, child_combiner<Combiners>(right_child_combiner),
	                    edge_val),
	 ...);
	return false;
}

template <class KeyT, class AggValueT, class... Combiners>
bool
CombinerPack<KeyT, AggValueT, Combiners...>::traverse_left_edge_up(
    KeyT new_point, AggValueT edge_val)
{
	// This is a fold expression with a comma operator!
	(std::get<Combiners>(this->data).traverse_left_edge_up(new_point, edge_val),
	 ...);
	return false;
}

template <class KeyT, class AggValueT, class... Combiners>
bool
CombinerPack<KeyT, AggValueT, Combiners...>::traverse_right_edge_up(
    KeyT new_point, AggValueT edge_val)
{
	// This is a fold expression with a comma operator!
	(std::get<Combiners>(this->data).traverse_right_edge_up(new_point, edge_val),
	 ...);
	return false;
}

template <class KeyT, class AggValueT, class... Combiners>
template <class Combiner>
typename Combiner::ValueT
CombinerPack<KeyT, AggValueT, Combiners...>::get() const
{
	return std::get<Combiner>(this->data).get();
}

template <class KeyT, class AggValueT, class... Combiners>
template <class Combiner>
const Combiner *
CombinerPack<KeyT, AggValueT, Combiners...>::child_combiner(
    const CombinerPack::MyType * child) const
{
	if (child == nullptr) {
		return nullptr;
	}

	return &child->get_combiner<Combiner>();
}

template <class KeyT, class AggValueT, class... Combiners>
template <class Combiner>
const Combiner &
CombinerPack<KeyT, AggValueT, Combiners...>::get_combiner() const
{
	return std::get<Combiner>(this->data);
}

} // namespace ygg

#endif
