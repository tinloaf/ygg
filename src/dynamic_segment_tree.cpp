//
// Created by lukas on 16.11.17.
//

#include "dynamic_segment_tree.hpp"
#include "debug.hpp"

#include <iostream>
#include <tuple>

namespace ygg {

namespace dyn_segtree_internal {

template<class InnerTree, class InnerNode>
void
InnerNodeTraits<InnerTree, InnerNode>::leaf_inserted(InnerNode & node){
	////std::cout << "### Leaf inserted: " << node.point << "\n";
}

template<class InnerTree, class InnerNode>
void
InnerNodeTraits<InnerTree, InnerNode>::delete_leaf(InnerNode & node)
{
	//std::cout << "### Deleting leaf " << node.point << "\n";

	// TODO DEBUG
	assert(node.agg_left == node.agg_right);

	InnerNode * parent = InnerTree::get_parent(&node);
	if (parent != nullptr) {
		if (InnerTree::get_left_child(parent) == &node) {
			parent->agg_left += node.agg_left;
		} else {
			// TODO DEBUG
			assert(InnerTree::get_right_child(parent) == &node);
			parent->agg_right += node.agg_left;
		}
	}
}

template<class InnerTree, class InnerNode>
void
InnerNodeTraits<InnerTree, InnerNode>::rotated_left(InnerNode & node)
{
	//std::cout << "### Rotating left around " << node.point << "\n";

	// TODO FIXME DEBUG

	InnerNode * old_right = InnerTree::get_parent(&node);

	typename InnerNode::AggValueT old_right_agg = node.agg_right;
	node.agg_right += old_right->agg_left;

	old_right->agg_left = typename InnerNode::AggValueT();
	old_right->agg_right += old_right_agg;

	InnerNode * root = debug::dbg_find_root(&node);
	debug::TreePrinter<InnerNode,
	                   typename dyn_segtree_internal::template InnerNodeNameGetter<InnerNode>>	tp(
					root, typename dyn_segtree_internal::InnerNodeNameGetter<InnerNode>());
	//tp.print();

}

template<class InnerTree, class InnerNode>
void
InnerNodeTraits<InnerTree, InnerNode>::rotated_right(InnerNode & node)
{
	//std::cout << "### Rotating right around " << node.point << "\n";
	// TODO FIXME DEBUG

	InnerNode * old_left = InnerTree::get_parent(&node);

	typename InnerNode::AggValueT old_left_agg = node.agg_left;
	node.agg_left += old_left->agg_right;

	old_left->agg_right = typename InnerNode::AggValueT();

	old_left->agg_left += old_left_agg;

	InnerNode * root = debug::dbg_find_root(&node);
	debug::TreePrinter<InnerNode,
	                   typename dyn_segtree_internal::template InnerNodeNameGetter<InnerNode>>	tp(
					root, typename dyn_segtree_internal::InnerNodeNameGetter<InnerNode>());
	//tp.print();
}

template<class InnerTree, class InnerNode>
void
InnerNodeTraits<InnerTree, InnerNode>::swapped(InnerNode & old_ancestor, InnerNode & old_descendant)
{
	//std::cout << "Swapped " << old_ancestor.point << " and " << old_descendant.point << "\n";

	// Swap labels to their old places in the tree
	std::swap(old_ancestor.InnerNode::agg_left, old_descendant.InnerNode::agg_left);
	std::swap(old_ancestor.InnerNode::agg_right, old_descendant.InnerNode::agg_right);

	if (old_ancestor.InnerNode::partner == &old_descendant) {
		// we are done. They have their contour nulled
		return;
	}

	// Unapply the contour between where the old descendant war and its partner
	InnerNode * old_descendant_partner = old_descendant.InnerNode::partner;


	if (old_descendant.InnerNode::point < old_descendant_partner->InnerNode::point) {
		InnerTree::modify_contour(&old_ancestor, old_descendant_partner, -1 * old_descendant.val);
		InnerTree::modify_contour(&old_descendant, old_descendant_partner, old_descendant.val);
	} else {
		assert(old_descendant.InnerNode::point > old_descendant_partner->InnerNode::point); // TODO
		// empty intervals break this currently
		InnerTree::modify_contour(old_descendant_partner, &old_ancestor, -1 * old_descendant.val);
		InnerTree::modify_contour(old_descendant_partner, &old_descendant, old_descendant.val);
	}
}

} // namespace dyn_segtree_internal


template <class Node, class NodeTraits, class Options, class Tag>
void
DynamicSegmentTree<Node, NodeTraits, Options, Tag>::insert(Node &n)
{
	// TODO remove this requirement?
	assert(NodeTraits::get_lower(n) < NodeTraits::get_upper(n));

	// TODO why are we doing this every time? Should be done once in the constructor!

	////std::cout << "========================================\n";
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
	////std::cout << "========================================\n";
}

template <class Node, class NodeTraits, class Options, class Tag>
void
DynamicSegmentTree<Node, NodeTraits, Options, Tag>::remove(Node &n)
{
	this->unapply_interval(n);

	//std::cout << "===== Tree before first remove (" << n.NB::start.point << "): \n";
	debug::TreePrinter<InnerNode,
	                   typename dyn_segtree_internal::template InnerNodeNameGetter<InnerNode>>	tp(
					this->t.get_root(), typename dyn_segtree_internal::InnerNodeNameGetter<InnerNode>());
	//tp.print();

	this->t.remove(n.NB::start);

	//std::cout << "===== Tree after first remove (" << n.NB::start.point << "): \n";
	tp.reset_root(this->t.get_root());
	//tp.print();

	this->t.remove(n.NB::end);

	//std::cout << "===== Tree after second remove (" << n.NB::end.point << "): \n";
	tp.reset_root(this->t.get_root());
	//tp.print();
}

template <class Node, class NodeTraits, class Options, class Tag>
typename DynamicSegmentTree<Node, NodeTraits, Options, Tag>::InnerTree::Contour
DynamicSegmentTree<Node, NodeTraits, Options, Tag>::InnerTree::find_lca(InnerNode *left, InnerNode *right)
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

template <class Node, class NodeTraits, class Options, class Tag>
void
DynamicSegmentTree<Node, NodeTraits, Options, Tag>::apply_interval(Node &n)
{
	InnerTree::modify_contour(&n.NB::start, &n.NB::end, NodeTraits::get_value(n));
}

template <class Node, class NodeTraits, class Options, class Tag>
void
DynamicSegmentTree<Node, NodeTraits, Options, Tag>::unapply_interval(Node &n)
{
	InnerTree::modify_contour(&n.NB::start, &n.NB::end, -1 * NodeTraits::get_value(n));
}

template <class Node, class NodeTraits, class Options, class Tag>
void
DynamicSegmentTree<Node, NodeTraits, Options, Tag>::InnerTree::modify_contour(InnerNode *left,
                                                                              InnerNode *right,
                                                                              ValueT val)
{
	std::vector<InnerNode *> left_contour;
	std::vector<InnerNode *> right_contour;
	std::tie(left_contour, right_contour) = DynamicSegmentTree<Node, NodeTraits, Options,
	                                                           Tag>::InnerTree::find_lca(left, right);


	//std::cout << "Tree: \n";
	debug::TreePrinter<InnerNode,
	                   typename dyn_segtree_internal::template InnerNodeNameGetter<InnerNode>>	tp(
					debug::dbg_find_root(left),
					typename dyn_segtree_internal::InnerNodeNameGetter<InnerNode>());
	//tp.print();

	//std::cout << "==> Applying interval: " << left->point << " -> " << right->point << " @ " << val << "\n";

	// left contour
	//std::cout << "Left contour: ";
	for (size_t i = 0 ; i < left_contour.size() - 1 ; ++i) {
		InnerNode * cur = left_contour[i];
		//std::cout << " [" << cur->InnerNode::point << "] ";
		if ((i == 0) || (InnerTree::get_right_child(cur) != left_contour[i-1])) {
			cur->InnerNode::agg_right += val;
		}
	}

	// right contour
	//std::cout <<"\nRight contour: ";
	for (size_t i = 0 ; i < right_contour.size() - 1 ; ++i) {
		InnerNode * cur = right_contour[i];
		//std::cout << " [" << cur->InnerNode::point << "] ";
		if ((i == 0) || (InnerTree::get_left_child(cur) != right_contour[i-1])) {
			cur->InnerNode::agg_left += val;
		}
	}

	//std::cout << "\n\n";
	tp.reset_root(debug::dbg_find_root(left));
	//tp.print();
}

template <class Node, class NodeTraits, class Options, class Tag>
typename Node::AggValueT
DynamicSegmentTree<Node, NodeTraits, Options, Tag>::query(const typename Node::KeyT & x)
{

	//std::cout << "\nQuery in Tree at " << std::to_string(x) << ": \n";
	debug::TreePrinter<InnerNode,
	                   typename dyn_segtree_internal::template InnerNodeNameGetter<InnerNode>>	tp(
					this->t.get_root(), typename dyn_segtree_internal::InnerNodeNameGetter<InnerNode>());
	//tp.print();

	InnerNode *cur = this->t.get_root();
	AggValueT agg = AggValueT();

	dyn_segtree_internal::Compare<InnerNode> cmp;

	while (cur != nullptr) {
		if (cmp(x, *cur)) {
			agg += cur->agg_left;
			//std::cout << "Left. Agg now " << agg << "\n";
			cur = InnerTree::get_left_child(cur);
		} else {
			agg += cur->agg_right;
			//std::cout << "Right. Agg now " << agg << "\n";
			cur = InnerTree::get_right_child(cur);
		}
	}

	return agg;
}


} // namespace ygg