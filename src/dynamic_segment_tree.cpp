//
// Created by lukas on 16.11.17.
//

#include "dynamic_segment_tree.hpp"
#include "util.hpp"

#include "debug.hpp"

#include <iostream>
#include <tuple>
#include <algorithm>

namespace ygg {

namespace dyn_segtree_internal {

template<class InnerTree, class InnerNode>
void
InnerNodeTraits<InnerTree, InnerNode>::leaf_inserted(InnerNode & node){
	(void) node;
}

template<class InnerTree, class InnerNode>
void
InnerNodeTraits<InnerTree, InnerNode>::delete_leaf(InnerNode & node)
{
	// TODO DEBUG
	//assert(node.agg_left == node.agg_right);

	InnerNode * parent = InnerTree::get_parent(&node);
	if (parent != nullptr) {
		if (InnerTree::get_left_child(parent) == &node) {
			parent->agg_left += node.agg_left;
		} else {
			// TODO DEBUG
			//assert(InnerTree::get_right_child(parent) == &node);
			parent->agg_right += node.agg_left;
		}
	}
}

template<class InnerTree, class InnerNode>
void
InnerNodeTraits<InnerTree, InnerNode>::rotated_left(InnerNode & node)
{
	InnerNode *old_right = InnerTree::get_parent(&node);

	typename InnerNode::AggValueT old_right_agg = node.agg_right;
	node.agg_right += old_right->agg_left;

	old_right->agg_left = typename InnerNode::AggValueT();
	old_right->agg_right += old_right_agg;

	InnerTree::rebuild_combiners_at(&node);
	InnerTree::rebuild_combiners_at(old_right);
}

template<class InnerTree, class InnerNode>
void
InnerNodeTraits<InnerTree, InnerNode>::rotated_right(InnerNode & node)
{
	InnerNode * old_left = InnerTree::get_parent(&node);

	typename InnerNode::AggValueT old_left_agg = node.agg_left;
	node.agg_left += old_left->agg_right;

	old_left->agg_right = typename InnerNode::AggValueT();
	old_left->agg_left += old_left_agg;

	InnerTree::rebuild_combiners_at(&node);
	InnerTree::rebuild_combiners_at(old_left);
}

template<class InnerTree, class InnerNode>
void
InnerNodeTraits<InnerTree, InnerNode>::swapped(InnerNode & old_ancestor, InnerNode & old_descendant)
{
	// Swap labels to their old places in the tree
	std::swap(old_ancestor.InnerNode::agg_left, old_descendant.InnerNode::agg_left);
	std::swap(old_ancestor.InnerNode::agg_right, old_descendant.InnerNode::agg_right);

	if (old_ancestor.InnerNode::partner == &old_descendant) {
		// we are done. They have their contour nulled
		InnerTree::rebuild_combiners_at(&old_ancestor);
		InnerTree::rebuild_combiners_at(&old_descendant); // TODO is this necessary?
		return;
	}

	// Unapply the contour between where the old descendant war and its partner
	InnerNode * old_descendant_partner = old_descendant.InnerNode::partner;

	if (old_descendant.InnerNode::point < old_descendant_partner->InnerNode::point) {
		InnerTree::modify_contour(&old_ancestor, old_descendant_partner, -1 * old_descendant.val);
		InnerTree::modify_contour(&old_descendant, old_descendant_partner, old_descendant.val);
	} else {
		// TODO
		// empty intervals break this currently
		InnerTree::modify_contour(old_descendant_partner, &old_ancestor, -1 * old_descendant.val);
		InnerTree::modify_contour(old_descendant_partner, &old_descendant, old_descendant.val);
	}
}

} // namespace dyn_segtree_internal


template <class Node, class NodeTraits, class Combiners, class Options, class Tag>
void
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, Tag>::insert(Node &n)
{
	// TODO remove this requirement?
	assert(NodeTraits::get_lower(n) < NodeTraits::get_upper(n));

	// TODO why are we doing this every time? Should be done once in the constructor!

	n.NB::start.val = NodeTraits::get_value(n);
	n.NB::start.point = NodeTraits::get_lower(n);
	n.NB::start.agg_left = AggValueT();
	n.NB::start.agg_right = AggValueT();

	n.NB::start.start = true;
	n.NB::start.partner = &n.NB::end;

	this->t.insert(n.NB::start);

	n.NB::end.val = NodeTraits::get_value(n);
	n.NB::end.point = NodeTraits::get_upper(n);
	n.NB::end.agg_left = AggValueT();
	n.NB::end.agg_right = AggValueT();

	n.NB::end.start = false;
	n.NB::end.partner = &n.NB::start;

	this->t.insert(n.NB::end);

	this->apply_interval(n);
}

template <class Node, class NodeTraits, class Combiners, class Options, class Tag>
void
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, Tag>::remove(Node &n)
{
	this->unapply_interval(n);

	this->t.remove(n.NB::start);
	this->t.remove(n.NB::end);
}

template <class Node, class NodeTraits, class Combiners, class Options, class Tag>
typename DynamicSegmentTree<Node, NodeTraits, Combiners, Options, Tag>::InnerTree::Contour
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, Tag>::InnerTree::find_lca(InnerNode *left, InnerNode *right)
{
	// TODO speed this up when nodes can be mapped to integers
	// TODO nonsense! Put a flag into the nodes!
	std::set<InnerNode *> left_set;
	std::set<InnerNode *> right_set;

	std::vector<InnerNode *> left_path;
	std::vector<InnerNode *> right_path;

	InnerNode * lca = nullptr;

	left_path.push_back(left);
	right_path.push_back(right);

	if (left == right) {
		return {left_path, right_path};
	}

	left_set.insert(left);
	right_set.insert(right);

	bool progress_left = true;
	while (true) {
		if (progress_left) {
			left = InnerTree::get_parent(left);

			if (left != nullptr) {
				left_path.push_back(left);
			}

			if (right_set.find(left) != right_set.end()) {
				lca = left;
				break;
			} else {
				left_set.insert(left);
			}
		} else {
			right = InnerTree::get_parent(right);

			if (right != nullptr) {
				right_path.push_back(right);
			}

			if (left_set.find(right) != left_set.end()) {
				lca = right;
				break;
			} else {
				right_set.insert(right);
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

	while (left_path.back() != lca) {
		left_path.pop_back();
	}
	while (right_path.back() != lca) {
		right_path.pop_back();
	}

	return {left_path, right_path};
}

template <class Node, class NodeTraits, class Combiners, class Options, class Tag>
void
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, Tag>::apply_interval(Node &n)
{
	InnerTree::modify_contour(&n.NB::start, &n.NB::end, NodeTraits::get_value(n));
}

template <class Node, class NodeTraits, class Combiners, class Options, class Tag>
void
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, Tag>::unapply_interval(Node &n)
{
	InnerTree::modify_contour(&n.NB::start, &n.NB::end, -1 * NodeTraits::get_value(n));
}

template <class Node, class NodeTraits, class Combiners, class Options, class Tag>
void
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, Tag>::InnerTree::modify_contour(InnerNode *left,
                                                                              InnerNode *right,
                                                                              ValueT val)
{
	std::vector<InnerNode *> left_contour;
	std::vector<InnerNode *> right_contour;
	std::tie(left_contour, right_contour) = find_lca(left, right);

	// left contour
	bool last_changed_left = false;
	for (size_t i = 0 ; i < left_contour.size() - 1 ; ++i) {
		InnerNode * cur = left_contour[i];
		if ((i == 0) || (InnerTree::get_right_child(cur) != left_contour[i-1])) {
			cur->InnerNode::agg_right += val;
			last_changed_left = rebuild_combiners_at(cur);
		}
	}

	// right contour
	bool last_changed_right = false;
	for (size_t i = 0 ; i < right_contour.size() - 1 ; ++i) {
		InnerNode * cur = right_contour[i];
		if ((i == 0) || (InnerTree::get_left_child(cur) != right_contour[i-1])) {
			cur->InnerNode::agg_left += val;
			last_changed_right = rebuild_combiners_at(cur);
		}
	}

	if (last_changed_left || last_changed_right) {
		InnerNode * lca = left_contour.size() > 0 ? left_contour[left_contour.size() - 1] :
		             right_contour[right_contour.size() - 1];
		rebuild_combiners_recursively(lca);
	}
}

template <class Node, class NodeTraits, class Combiners, class Options, class Tag>
typename Node::AggValueT
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, Tag>::query(const typename Node::KeyT & x)
{
	InnerNode *cur = this->t.get_root();
	AggValueT agg = AggValueT();

	dyn_segtree_internal::Compare<InnerNode> cmp;

	while (cur != nullptr) {
		if (cmp(x, *cur)) {
			agg += cur->agg_left;
			cur = InnerTree::get_left_child(cur);
		} else {
			agg += cur->agg_right;
			cur = InnerTree::get_right_child(cur);
		}
	}

	return agg;
}


template <class Node, class NodeTraits, class Combiners, class Options, class Tag>
bool
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, Tag>::InnerTree::
				rebuild_combiners_at(InnerNode *n)
{
	Combiners * cmb_left = nullptr;
	if (n->_rbt_left != nullptr) {
		cmb_left = & n->_rbt_left->combiners;
	}
	Combiners * cmb_right = nullptr;
	if (n->_rbt_right != nullptr) {
		cmb_right = & n->_rbt_right->combiners;
	}
	return n->combiners.rebuild(cmb_left, n->agg_left, cmb_right, n->agg_right);
}

template <class Node, class NodeTraits, class Combiners, class Options, class Tag>
void
DynamicSegmentTree<Node, NodeTraits, Combiners, Options, Tag>::InnerTree::
				rebuild_combiners_recursively(InnerNode *n)
{
	Combiners * cmb_left = nullptr;
	if (n->_rbt_left != nullptr) {
		cmb_left = & n->_rbt_left->combiners;
	}
	Combiners * cmb_right = nullptr;
	if (n->_rbt_right != nullptr) {
		cmb_right = & n->_rbt_right->combiners;
	}

	while(n->InnerNode::combiners.rebuild(cmb_left, n->agg_left, cmb_right, n->agg_right)) {
		if (n != nullptr) {
			n = n->_rbt_parent;

			if (n->_rbt_left != nullptr) {
				cmb_left = & n->_rbt_left->combiners;
			} else {
				cmb_left = nullptr;
			}
			if (n->_rbt_right != nullptr) {
				cmb_right = & n->_rbt_right->combiners;
			} else {
				cmb_right = nullptr;
			}
		} else {
			break;
		}
	}
}

template<class Node>
void
MaxCombiner<Node>::combine_with(typename Node::AggValueT a)
{
	this->val = std::max(this->val, a);
}

template<class Node>
typename MaxCombiner<Node>::ValueT
MaxCombiner<Node>::get()
{
	return this->val;
}

template<class Node>
bool
MaxCombiner<Node>::rebuild(typename Node::AggValueT a, typename Node::AggValueT a_edge_val,
                           typename Node::AggValueT b, typename Node::AggValueT b_edge_val)
{
	auto old_val = this->val;
	this->val = std::max(a + a_edge_val, b + b_edge_val);
	return old_val != val;
}

template<class Node>
MaxCombiner<Node>::MaxCombiner(typename Node::AggValueT val_in)
	: val(val_in)
{}

template<class AggValueT, class ... Combiners>
CombinerPack<AggValueT, Combiners...>::CombinerPack(AggValueT val)
	: data { Combiners(val) ... }
{}

template<class AggValueT, class ... Combiners>
bool
CombinerPack<AggValueT, Combiners...>::rebuild(CombinerPack<AggValueT, Combiners...> * a,
                                          AggValueT a_edge_val,
                                          CombinerPack<AggValueT, Combiners...> * b,
                                          AggValueT b_edge_val)
{
	// TODO replace vector by templated std::any_of?
	std::vector<bool> changed { std::get<Combiners>(this->data).rebuild(
																	a != nullptr ? a->get<Combiners>() : AggValueT(),
																	a_edge_val,
																	b != nullptr ? b->get<Combiners>() : AggValueT())
					                            ...};
	return std::any_of(changed.begin(), changed.end(), [](bool b) { return b ;});
}

template<class AggValueT, class ... Combiners>
void
CombinerPack<AggValueT, Combiners...>::combine_with(CombinerPack<AggValueT, Combiners...> *other)
{
	utilities::throw_away(std::get<Combiners>(this->data).combine_with(other->get<Combiners>()) ...);
}

template<class Node, class ... Combiners>
template<class Combiner>
typename Combiner::ValueT
CombinerPack<Node, Combiners...>::get()
{
	return std::get<Combiner>(this->data).get();
}

} // namespace ygg