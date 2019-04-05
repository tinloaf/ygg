#ifndef YGG_RBTREE_CPP
#define YGG_RBTREE_CPP

#include "rbtree.hpp"

namespace ygg {

namespace rbtree_internal {

template <class Node>
void
ColorParentStorage<Node, true>::set_color(Color new_color)
{
	if (new_color == Color::RED) {
		this->parent = (Node *)((size_t)this->parent | 1);
	} else {
		this->parent = (Node *)((size_t)this->parent & ~((size_t)1));
	}
}

template <class Node>
ygg::rbtree_internal::Color
ColorParentStorage<Node, true>::get_color() const
{
	if ((size_t)this->parent & 1) {
		return Color::RED;
	} else {
		return Color::BLACK;
	}
}

template <class Node>
void
ColorParentStorage<Node, true>::set_parent(Node * new_parent)
{
	this->parent = (Node *)((size_t)new_parent | ((size_t)this->parent & 1));
}

template <class Node>
Node *
ColorParentStorage<Node, true>::get_parent() const
{
	return (Node *)((size_t)this->parent & (~((size_t)1)));
}

template <class Node>
void
ColorParentStorage<Node, true>::swap_color_with(
    ColorParentStorage<Node, true> & other)
{
	// TODO make this more efficient?
	Color tmp = other.get_color();
	other.set_color(this->get_color());
	this->set_color(tmp);
}

template <class Node>
void
ColorParentStorage<Node, true>::swap_parent_with(
    ColorParentStorage<Node, true> & other)
{
	// TODO make this more efficient?
	Node * tmp = other.get_parent();
	other.set_parent(this->get_parent());
	this->set_parent(tmp);
}

// TODO have a 'swap both' operator!

template <class Node>
void
ColorParentStorage<Node, false>::set_color(Color new_color)
{
	this->color = new_color;
}

template <class Node>
ygg::rbtree_internal::Color
ColorParentStorage<Node, false>::get_color() const
{
	return this->color;
}

template <class Node>
void
ColorParentStorage<Node, false>::set_parent(Node * new_parent)
{
	this->parent = new_parent;
}

template <class Node>
Node *
ColorParentStorage<Node, false>::get_parent() const
{
	return this->parent;
}

template <class Node>
void
ColorParentStorage<Node, false>::swap_color_with(
    ColorParentStorage<Node, false> & other)
{
	std::swap(this->color, other.color);
}

template <class Node>
void
ColorParentStorage<Node, false>::swap_parent_with(
    ColorParentStorage<Node, false> & other)
{
	std::swap(this->parent, other.parent);
}
} // namespace rbtree_internal

template <class Node, class Tag, class Options>
void
RBTreeNodeBase<Node, Tag, Options>::set_color(rbtree_internal::Color new_color)
{
	this->_bst_parent.set_color(new_color);
}

template <class Node, class Tag, class Options>
rbtree_internal::Color
RBTreeNodeBase<Node, Tag, Options>::get_color() const
{
	return this->_bst_parent.get_color();
}

template <class Node, class Tag, class Options>
void
RBTreeNodeBase<Node, Tag, Options>::swap_color_with(Node * other)
{
	this->_bst_parent.swap_color_with(other->_bst_parent);
}

template <class Node, class Tag, class Options>
void
RBTreeNodeBase<Node, Tag, Options>::swap_parent_with(Node * other)
{
	this->_bst_parent.swap_parent_with(other->_bst_parent);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
RBTree<Node, NodeTraits, Options, Tag, Compare>::RBTree()
{}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
RBTree<Node, NodeTraits, Options, Tag, Compare>::RBTree(MyClass && other)
{
	this->root = other.root;
	other.root = nullptr;
	this->s = other.s;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <bool on_equality_prefer_left>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::insert_leaf_base(Node & node,
                                                                  Node * start)
{
	node.NB::set_right(nullptr);
	node.NB::set_left(nullptr);

	Node * parent = start;
	Node * cur = start;

	while (cur != nullptr) {
		parent = cur;

		if constexpr (Options::multiple) {
			if constexpr (on_equality_prefer_left) {
				if (this->cmp(*cur, node)) {
					cur = cur->NB::get_right();
				} else {
					cur = cur->NB::get_left();
				}
			} else {
				if (this->cmp(node, *cur)) {
					cur = cur->NB::get_left();
				} else {
					cur = cur->NB::get_right();
				}
			}
		} else {
			// Multiple are not allowed - we need three-way comparisons!
			// on_equality_prefer_left has no effect here
			if (this->cmp(*cur, node)) {
				cur = cur->NB::get_right();
			} else if (this->cmp(node, *cur)) {
				cur = cur->NB::get_left();
			} else {
				// Same as existing. Reduce size (because we increased it earlier)
				// and exit.
				this->s.reduce(1);
				return;
			}
		}
	}

	if (parent == nullptr) {
		// new root!
		node.NB::set_parent(nullptr);
		node.NB::set_color(rbtree_internal::Color::BLACK);
		this->root = &node;
		NodeTraits::leaf_inserted(node, *this);
	} else {
		node.NB::set_parent(parent);
		node.NB::set_color(rbtree_internal::Color::RED);

		// TODO if multiple are allowed, we can make this a two-way comparison!
		if (this->cmp(node, *parent)) {
			parent->NB::set_left(&node);
		} else if (this->cmp(*parent, node)) {
			parent->NB::set_right(&node);
		} else {
			// assert(multiple);

			// TODO constexpr - if
			if constexpr (!Options::multiple) {
				// We already added to the size, subtract it again!
				this->s.reduce(1);
				return;
			} else {

				// TODO constexpr - if
				if constexpr (on_equality_prefer_left) {
					parent->NB::set_left(&node);
				} else {
					parent->NB::set_right(&node);
				}
			}
		}

		NodeTraits::leaf_inserted(node, *this);
		this->fixup_after_insert(&node);
	}

	return;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::rotate_left(Node * parent)
{
	Node * right_child = parent->NB::get_right();
	parent->NB::set_right(right_child->NB::get_left());
	if (right_child->NB::get_left() != nullptr) {
		right_child->NB::get_left()->NB::set_parent(parent);
	}

	Node * parents_parent = parent->NB::get_parent();

	right_child->NB::set_left(parent);
	right_child->NB::set_parent(parents_parent);

	if (parents_parent != nullptr) {
		if (parents_parent->NB::get_left() == parent) {
			parents_parent->NB::set_left(right_child);
		} else {
			parents_parent->NB::set_right(right_child);
		}
	} else {
		this->root = right_child;
	}

	parent->NB::set_parent(right_child);

	NodeTraits::rotated_left(*parent, *this);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::rotate_right(Node * parent)
{
	Node * left_child = parent->NB::get_left();
	parent->NB::set_left(left_child->NB::get_right());
	if (left_child->NB::get_right() != nullptr) {
		left_child->NB::get_right()->NB::set_parent(parent);
	}

	Node * parents_parent = parent->NB::get_parent();

	left_child->NB::set_right(parent);
	left_child->NB::set_parent(parents_parent);

	if (parents_parent != nullptr) {
		if (parents_parent->NB::get_left() == parent) {
			parents_parent->NB::set_left(left_child);
		} else {
			parents_parent->NB::set_right(left_child);
		}
	} else {
		this->root = left_child;
	}

	parent->NB::set_parent(left_child);

	NodeTraits::rotated_right(*parent, *this);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::fixup_after_insert(Node * node)
{
	// Does not happen: We only call this if we are not the root.
	/*
	if (node->NB::get_parent() == nullptr) {
	  node->NB::set_color(rbtree_internal::Color::BLACK);
	  return;
	}
	*/

	while (
	    (node->NB::get_parent()->NB::get_color() ==
	     rbtree_internal::Color::RED) &&
	    (this->get_uncle(node) != nullptr) &&
	    (this->get_uncle(node)->NB::get_color() == rbtree_internal::Color::RED)) {
		Node * parent = node->NB::get_parent();
		parent->NB::set_color(rbtree_internal::Color::BLACK);
		this->get_uncle(node)->NB::set_color(rbtree_internal::Color::BLACK);

		Node * grandparent = parent->NB::get_parent();
		if (grandparent->NB::get_parent() !=
		    nullptr) { // never iterate into the root
			grandparent->NB::set_color(rbtree_internal::Color::RED);
			node = grandparent;
		} else {
			// Don't recurse into the root; don't color it red. We could immediately
			// re-color it black.
			return;
		}
	}

	if (node->NB::get_parent()->NB::get_color() ==
	    rbtree_internal::Color::BLACK) {
		return;
	}

	Node * parent = node->NB::get_parent();
	Node * grandparent = parent->NB::get_parent();

	if (grandparent->NB::get_left() == parent) {
		if (parent->NB::get_right() == node) {
			// 'folded in' situation
			this->rotate_left(parent);
			node->NB::set_color(rbtree_internal::Color::BLACK);
		} else {
			// 'straight' situation
			parent->NB::set_color(rbtree_internal::Color::BLACK);
		}

		this->rotate_right(grandparent);
	} else {
		if (parent->NB::get_left() == node) {
			// 'folded in'
			this->rotate_right(parent);
			node->NB::set_color(rbtree_internal::Color::BLACK);
		} else {
			// 'straight'
			parent->NB::set_color(rbtree_internal::Color::BLACK);
		}
		this->rotate_left(grandparent);
	}

	grandparent->NB::set_color(rbtree_internal::Color::RED);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::insert(Node & node)
{
	this->s.add(1);
	this->insert_leaf_base<true>(node, this->root);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::insert_left_leaning(
    Node & node)
{
	this->s.add(1);
	this->insert_leaf_base<true>(node, this->root);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::insert_right_leaning(
    Node & node)
{
	this->s.add(1);
	this->insert_leaf_base<false>(node, this->root);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::insert(Node & node,
                                                        Node & hint)
{
	/* TODO this code does not work. We need to traverse the path up until
	 * we have seen at least one smaller-than and one larger-than node.
	 * Is this really faster? For now, fall back to normal insertion.
	 */
	(void)hint;
	this->insert(node);

	// find parent
	// Node * parent = &hint;

	/* We need to walk up if:
	 *  - we're larger than the parent and in its left subtree
	 *  - we're smaller than the parent and in its right subtree
	 */
	/*
	while (
	    (parent->NB::get_parent() != nullptr) &&
	    (((parent->NB::get_parent()->NB::get_left() == parent) &&
	      (this->cmp(*parent->NB::get_parent(),
	                 node))) || // left subtree, parent should go before node
	     ((parent->NB::get_parent()->NB::get_right() == parent) &&
	      (this->cmp(node,
	                 *parent->NB::get_parent()))))) { // right subtree, node
	                                                  // should go before parent
	  parent = parent->NB::get_parent();
	}

	if (parent->NB::get_left() != nullptr) {
	  parent = parent->NB::get_left();
	  this->insert_leaf_base<false>(node, parent);
	} else {
	  this->insert_leaf_base<true>(node, parent);
	}
	*/
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::insert(
    Node & node,
    RBTree<Node, NodeTraits, Options, Tag, Compare>::iterator<false> hint)
{
	this->insert(node);
	/* TODO see above.*/
	/*
	if (hint == this->end()) {
	  // special case: insert at the end
	  Node * parent = this->root;

	  if (parent == nullptr) {
	    this->insert_leaf_base<false>(node, parent);
	  } else {
	    while (parent->NB::get_right() != nullptr) {
	      parent = parent->NB::get_right();
	    }
	    this->insert_leaf_base<false>(node, parent);
	  }
	} else {
	  this->insert(node, *hint);
	}
	*/
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
bool
RBTree<Node, NodeTraits, Options, Tag, Compare>::verify_black_root() const
{
	return ((this->root == nullptr) ||
	        (this->root->NB::get_color() == rbtree_internal::Color::BLACK));
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
bool
RBTree<Node, NodeTraits, Options, Tag, Compare>::verify_black_paths(
    const Node * node, unsigned int * path_length) const
{
	unsigned int left_length, right_length;

	if (node->NB::get_left() == nullptr) {
		left_length = 0;
	} else {
		if (!this->verify_black_paths(node->NB::get_left(), &left_length)) {
			return false;
		}
	}

	if (node->NB::get_right() == nullptr) {
		right_length = 0;
	} else {
		if (!this->verify_black_paths(node->NB::get_right(), &right_length)) {
			return false;
		}
	}

	if (left_length != right_length) {
		return false;
	}

	if (node->NB::get_color() == rbtree_internal::Color::BLACK) {
		*path_length = left_length + 1;
	} else {
		*path_length = left_length;
	}

	return true;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
bool
RBTree<Node, NodeTraits, Options, Tag, Compare>::verify_red_black(
    const Node * node) const
{
	if (node == nullptr) {
		return true;
	}

	if (node->NB::get_color() == rbtree_internal::Color::RED) {
		if ((node->NB::get_right() != nullptr) &&
		    (node->NB::get_right()->NB::get_color() ==
		     rbtree_internal::Color::RED)) {
			return false;
		}

		if ((node->NB::get_left() != nullptr) &&
		    (node->NB::get_left()->NB::get_color() ==
		     rbtree_internal::Color::RED)) {
			return false;
		}
	}

	return this->verify_red_black(node->NB::get_left()) &&
	       this->verify_red_black(node->NB::get_right());
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
bool
RBTree<Node, NodeTraits, Options, Tag, Compare>::verify_integrity() const
{
	unsigned int dummy;

	bool paths_okay =
	    (this->root == nullptr) || this->verify_black_paths(this->root, &dummy);
	bool children_okay = this->verify_red_black(this->root);

	bool root_okay = this->verify_black_root();

	bool bst_okay = this->TB::verify_integrity();

	// TODO adapt to new verification scheme
	assert(root_okay && paths_okay && children_okay && bst_okay);

	(void)dummy;

	return (root_okay && paths_okay && children_okay && bst_okay);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::swap_nodes(Node * n1,
                                                            Node * n2,
                                                            bool swap_colors)
{
	if (n1->NB::get_parent() == n2) { // TODO this should never happen, since n2 is always the descendant
		this->swap_neighbors(n2, n1);
	} else if (n2->NB::get_parent() == n1) {
		this->swap_neighbors(n1, n2);
	} else {
		this->swap_unrelated_nodes(n1, n2);
	}

	if (!swap_colors) {
		n1->swap_color_with(n2);
	}

	NodeTraits::swapped(*n1, *n2, *this);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::replace_node(
    Node * to_be_replaced, Node * replace_with)
{
	Node * parent = to_be_replaced->get_parent();
	if (parent != nullptr) {
		if (parent->get_left() == to_be_replaced) {
			parent->NB::set_left(replace_with);
		} else {
			parent->NB::set_right(replace_with);
		}
	} else {
		this->root = replace_with;
	}
	replace_with->set_parent(parent);

	// TODO callback?
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::swap_neighbors(Node * parent,
                                                                Node * child)
{
	child->NB::set_parent(parent->NB::get_parent());
	parent->NB::set_parent(child);
	if (child->NB::get_parent() != nullptr) {
		if (child->NB::get_parent()->NB::get_left() == parent) {
			child->NB::get_parent()->NB::set_left(child);
		} else {
			child->NB::get_parent()->NB::set_right(child);
		}
	} else {
		this->root = child;
	}

	if (parent->NB::get_left() == child) {
		parent->NB::set_left(child->NB::get_left());
		if (parent->NB::get_left() != nullptr) {
			parent->NB::get_left()->NB::set_parent(parent);
		}
		child->NB::set_left(parent);

		std::swap(parent->NB::get_right(), child->NB::get_right());
		if (child->NB::get_right() != nullptr) {
			child->NB::get_right()->NB::set_parent(child);
		}
		if (parent->NB::get_right() != nullptr) {
			parent->NB::get_right()->NB::set_parent(parent);
		}
	} else {
		parent->NB::set_right(child->NB::get_right());
		if (parent->NB::get_right() != nullptr) {
			parent->NB::get_right()->NB::set_parent(parent);
		}
		child->NB::set_right(parent);

		std::swap(parent->NB::get_left(), child->NB::get_left());
		if (child->NB::get_left() != nullptr) {
			child->NB::get_left()->NB::set_parent(child);
		}
		if (parent->NB::get_left() != nullptr) {
			parent->NB::get_left()->NB::set_parent(parent);
		}
	}
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::swap_unrelated_nodes(Node * n1,
                                                                      Node * n2)
{
	std::swap(n1->NB::get_left(), n2->NB::get_left());
	if (n1->NB::get_left() != nullptr) {
		n1->NB::get_left()->NB::set_parent(n1);
	}
	if (n2->NB::get_left() != nullptr) {
		n2->NB::get_left()->NB::set_parent(n2);
	}

	std::swap(n1->NB::get_right(), n2->NB::get_right());
	if (n1->NB::get_right() != nullptr) {
		n1->NB::get_right()->NB::set_parent(n1);
	}
	if (n2->NB::get_right() != nullptr) {
		n2->NB::get_right()->NB::set_parent(n2);
	}

	n1->NB::swap_parent_with(n2);

	if (n1->NB::get_parent() != nullptr) {
		if (n1->NB::get_parent()->NB::get_right() == n2) {
			n1->NB::get_parent()->NB::set_right(n1);
		} else {
			n1->NB::get_parent()->NB::set_left(n1);
		}
	} else {
		this->root = n1;
	}
	if (n2->NB::get_parent() != nullptr) {
		if (n2->NB::get_parent()->NB::get_right() == n1) {
			n2->NB::get_parent()->NB::set_right(n2);
		} else {
			n2->NB::get_parent()->NB::set_left(n2);
		}
	} else {
		this->root = n2;
	}
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::remove_to_leaf(Node & node)
{
	Node * cur = &node;
	Node * child = &node;

	if ((cur->NB::get_right() != nullptr) && (cur->NB::get_left() != nullptr)) {
		// Find the minimum of the larger-or-equal children
		child = cur->NB::get_right();
		while (child->NB::get_left() != nullptr) {
			child = child->NB::get_left();
		}
	} else if (cur->NB::get_left() != nullptr) {
		// Only a left child. This must be red and cannot have further children
		// (otherwise, black-balance would be violated)
		child = child->NB::get_left();
	}

	if (child != &node) {
		this->swap_nodes(&node, child, false);
	}
	// Now, node is a pseudo-leaf with the color of child.

	// Node cannot have a left child, so if it has a right child, it must be red,
	// thus node must be black
	if (node.NB::get_right() != nullptr) {
		// replace node with its child and color the child black.
		auto right_child = node.NB::get_right();

		/*
		 * If the delete_leaf() callback is implemented, we need to actually swap
		 * the right child with the node to be deleted, s.t. we still have a node
		 * that we can call the callback on. If it is not implemented, we can just
		 * replace the node with its child, not caring to "swap back".
		 */
		// TODO C++17 mark this if constexpr
		// TODO fix this check for the new templates
		/*
		if (&NodeTraits::delete_leaf == &RBDefaultNodeTraits::delete_leaf<Node,
		MyClass>) {
		  // Not overridden
		  this->replace_node(&node, right_child);
		  right_child->NB::set_color(rbtree_internal::Color::BLACK);
		  } else {*/
		// Overridden
		this->swap_nodes(&node, right_child, true);

		NodeTraits::delete_leaf(node, *this);

		right_child->NB::set_color(rbtree_internal::Color::BLACK);
		right_child->NB::set_right(nullptr); // this stored the node to be deleted…
		                                     // TODO null the pointers in node?
		                                     //}

		NodeTraits::deleted_below(*right_child, *this);

		return; // no fixup necessary
	}

	// Node has no children, so we have to just delete it, which is no problem if
	// we are red. Otherwise, we must start a fixup at the parent.
	bool deleted_left = false;
	NodeTraits::delete_leaf(node, *this);
	if (node.NB::get_parent() != nullptr) {
		if (node.NB::get_parent()->NB::get_left() == &node) {
			node.NB::get_parent()->NB::set_left(nullptr);
			deleted_left = true;
		} else {
			node.NB::get_parent()->NB::set_right(nullptr);
		}

		NodeTraits::deleted_below(*node.NB::get_parent(), *this);
	} else {
		this->root = nullptr; // Tree is now empty!
		return;               // No fixup needed!
	}

	if (node.NB::get_color() == rbtree_internal::Color::BLACK) {
		this->fixup_after_delete(node.NB::get_parent(), deleted_left);
	}
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::fixup_after_delete(
    Node * parent, bool deleted_left)
{
	bool propagating_up = true;
	Node * sibling;

	while (propagating_up) {
		// We just deleted a black node from under parent.
		if (deleted_left) {
			sibling = parent->NB::get_right();
		} else {
			sibling = parent->NB::get_left();
		}

		// sibling must exist! If it didn't, then that branch would have had too few
		// blacks…
		if ((parent->NB::get_color() == rbtree_internal::Color::BLACK) &&
		    (sibling->NB::get_color() == rbtree_internal::Color::BLACK) &&
		    ((sibling->NB::get_left() == nullptr) ||
		     (sibling->NB::get_left()->NB::get_color() ==
		      rbtree_internal::Color::BLACK)) &&
		    ((sibling->NB::get_right() == nullptr) ||
		     (sibling->NB::get_right()->NB::get_color() ==
		      rbtree_internal::Color::BLACK))) {

			// We can recolor and propagate up! (Case 3)
			sibling->NB::set_color(rbtree_internal::Color::RED);
			// Now everything below parent is okay, but the branch started in parent
			// lost a black!
			if (parent->NB::get_parent() == nullptr) {
				// Doesn't matter! parent is the root, no harm done.
				return;
			} else {
				// propagate up!
				deleted_left = parent->NB::get_parent()->NB::get_left() == parent;
				parent = parent->NB::get_parent();
			}
		} else { // could not recolor the sibling, do not propagate up
			propagating_up = false;
		}
	}

	if (sibling->NB::get_color() == rbtree_internal::Color::RED) {
		// Case 2
		sibling->NB::set_color(rbtree_internal::Color::BLACK);
		parent->NB::set_color(rbtree_internal::Color::RED);
		if (deleted_left) {
			this->rotate_left(parent);
			sibling = parent->NB::get_right();
		} else {
			this->rotate_right(parent);
			sibling = parent->NB::get_left();
		}
	}

	if ((sibling->NB::get_color() == rbtree_internal::Color::BLACK) &&
	    ((sibling->NB::get_left() == nullptr) ||
	     (sibling->NB::get_left()->NB::get_color() ==
	      rbtree_internal::Color::BLACK)) &&
	    ((sibling->NB::get_right() == nullptr) ||
	     (sibling->NB::get_right()->NB::get_color() ==
	      rbtree_internal::Color::BLACK))) {
		// case 4
		parent->NB::set_color(rbtree_internal::Color::BLACK);
		sibling->NB::set_color(rbtree_internal::Color::RED);

		return; // No further fixup necessary
	}

	if (deleted_left) {
		if ((sibling->NB::get_right() == nullptr) ||
		    (sibling->NB::get_right()->NB::get_color() ==
		     rbtree_internal::Color::BLACK)) {
			// left child of sibling must be red! This is the folded case. (Case 5)
			// Unfold!
			this->rotate_right(sibling);
			sibling->NB::set_color(rbtree_internal::Color::RED);
			// The new sibling is now the parent of the sibling
			sibling = sibling->NB::get_parent();
			sibling->NB::set_color(rbtree_internal::Color::BLACK);
		}

		// straight situation, case 6 applies!
		this->rotate_left(parent);

		parent->NB::swap_color_with(sibling);

		sibling->NB::get_right()->NB::set_color(rbtree_internal::Color::BLACK);
	} else {
		if ((sibling->NB::get_left() == nullptr) ||
		    (sibling->NB::get_left()->NB::get_color() ==
		     rbtree_internal::Color::BLACK)) {
			// right child of sibling must be red! This is the folded case. (Case 5)
			// Unfold!

			this->rotate_left(sibling);
			sibling->NB::set_color(rbtree_internal::Color::RED);
			// The new sibling is now the parent of the sibling
			sibling = sibling->NB::get_parent();
			sibling->NB::set_color(rbtree_internal::Color::BLACK);
		}

		// straight situation, case 6 applies!
		this->rotate_right(parent);
		parent->NB::swap_color_with(sibling);
		sibling->NB::get_left()->NB::set_color(rbtree_internal::Color::BLACK);
	}
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::remove(Node & node)
{
	this->s.reduce(1);

	// TODO collapse this method
	this->remove_to_leaf(node);
}

} // namespace ygg

#endif // YGG_RBTREE_CPP
