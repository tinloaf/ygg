//
// Created by lukas on 16.11.17.
//

#include "dynamic_segment_tree.hpp"
#include "debug.hpp"

#include <tuple>

namespace ygg {

namespace dyn_segtree_internal {

template<class InnerTree, class InnerNode>
void
InnerNodeTraits<InnerTree, InnerNode>::leaf_inserted(InnerNode & node){
	//std::cout << "### Leaf inserted: " << node.point << "\n";
}

template<class InnerTree, class InnerNode>
void
InnerNodeTraits<InnerTree, InnerNode>::rotated_left(InnerNode & node)
{
	//std::cout << "### Rotating left around " << node.point << "\n";

	InnerNode * old_right = InnerTree::get_parent(&node);

	typename InnerNode::AggValueT old_right_agg = node.agg_right;
	node.agg_right += old_right->agg_left;

	old_right->agg_left = typename InnerNode::AggValueT();
	old_right->agg_right += old_right_agg;
}

template<class InnerTree, class InnerNode>
void
InnerNodeTraits<InnerTree, InnerNode>::rotated_right(InnerNode & node)
{
	//std::cout << "### Rotating right around " << node.point << "\n";

	InnerNode * old_left = InnerTree::get_parent(&node);

	typename InnerNode::AggValueT old_left_agg = node.agg_left;
	node.agg_left += old_left->agg_right;

	old_left->agg_right = typename InnerNode::AggValueT();
	old_left->agg_left += old_left_agg;
}

template<class InnerTree, class InnerNode>
void
InnerNodeTraits<InnerTree, InnerNode>::swapped(InnerNode & n1, InnerNode & n2)
{
}

} // namespace dyn_segtree_internal


template <class Node, class NodeTraits, class Options, class Tag>
void
DynamicSegmentTree<Node, NodeTraits, Options, Tag>::insert(Node &n)
{
	//std::cout << "========================================\n";
	n.NB::start.point = NodeTraits::get_lower(n);
	n.NB::start.agg_left = AggValueT();
	n.NB::start.agg_right = AggValueT();
	n.NB::start.start = true;

	this->t.insert(n.NB::start);

	/*
	std::cout << "===== Tree after first insert: \n";
	TreePrinter<Tree, InnerNode, typename dyn_segtree_internal::template InnerNodeNameGetter<InnerNode>> tp(
					&this->t, typename dyn_segtree_internal::InnerNodeNameGetter<InnerNode>());
	tp.print();
	 */

	n.NB::end.point = NodeTraits::get_upper(n);
	n.NB::end.agg_left = AggValueT();
	n.NB::end.agg_right = AggValueT();
	n.NB::end.start = false;

	this->t.insert(n.NB::end);

	/*
	std::cout << "===== Tree after second insert: \n";
	tp.print();

	std::cout << "===== Applying Interval";
	 */
	this->apply_interval(n);
	//std::cout << "========================================\n";
}

template <class Node, class NodeTraits, class Options, class Tag>
typename DynamicSegmentTree<Node, NodeTraits, Options, Tag>::Contour
DynamicSegmentTree<Node, NodeTraits, Options, Tag>::find_lca(InnerNode *left, InnerNode *right)
{
	// TODO speed this up when nodes can be mapped to integers
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
			left = Tree::get_parent(left);

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
			right = Tree::get_parent(right);

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
	std::vector<InnerNode *> left_contour;
	std::vector<InnerNode *> right_contour;
	std::tie(left_contour, right_contour) = this->find_lca(&n.NB::start, &n.NB::end);

	/*
	std::cout << "Tree: \n";
	TreePrinter<Tree, InnerNode, typename dyn_segtree_internal::template InnerNodeNameGetter<InnerNode>> tp(
					&this->t, typename dyn_segtree_internal::InnerNodeNameGetter<InnerNode>());
	tp.print();

	std::cout << "==> Applying interval: " << NodeTraits::get_lower(n) << " -> "
	          << NodeTraits::get_upper(n) << " @ " << NodeTraits::get_value(n) << "\n";
	*/

	auto val = NodeTraits::get_value(n);

	// left contour
	//std::cout << "Left contour: ";
	for (size_t i = 0 ; i < left_contour.size() - 1 ; ++i) {
		InnerNode * cur = left_contour[i];
		//std::cout << " [" << cur->InnerNode::point << "] ";
		if ((i == 0) || (Tree::get_right_child(cur) != left_contour[i-1])) {
			cur->InnerNode::agg_right += val;
		}
	}

	// right contour
	//std::cout <<"\nRight contour: ";
	for (size_t i = 0 ; i < right_contour.size() - 1 ; ++i) {
		InnerNode * cur = right_contour[i];
		//std::cout << " [" << cur->InnerNode::point << "] ";
		if ((i == 0) || (Tree::get_left_child(cur) != right_contour[i-1])) {
			cur->InnerNode::agg_left += val;
		}
	}

	//std::cout << "\n\n";
	//tp.print();
}

template <class Node, class NodeTraits, class Options, class Tag>
typename Node::AggValueT
DynamicSegmentTree<Node, NodeTraits, Options, Tag>::query(const typename Node::KeyT & x)
{
	/*
	std::cout << "\nQuery in Tree at " << std::to_string(x) << ": \n";
	TreePrinter<Tree, InnerNode, typename dyn_segtree_internal::template InnerNodeNameGetter<InnerNode>> tp(
					&this->t, typename dyn_segtree_internal::InnerNodeNameGetter<InnerNode>());
	tp.print();
*/
	InnerNode *cur = this->t.get_root();
	AggValueT agg = AggValueT();

	dyn_segtree_internal::Compare<InnerNode> cmp;

	while (cur != nullptr) {
		if (cmp(x, *cur)) {
			agg += cur->agg_left;
			//std::cout << "Left. Agg now " << agg << "\n";
			cur = Tree::get_left_child(cur);
		} else {
			agg += cur->agg_right;
			//std::cout << "Right. Agg now " << agg << "\n";
			cur = Tree::get_right_child(cur);
		}
	}

	return agg;
}


} // namespace ygg