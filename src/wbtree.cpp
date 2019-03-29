#ifndef YGG_WBTREE_CPP
#define YGG_WBTREE_CPP

#include "wbtree.hpp"

namespace ygg {
namespace weight {

template <class Node, class Tag, class Options>
void
WBTreeNodeBase<Node, Tag, Options>::swap_parent_with(Node * other)
{
	std::swap(this->_bst_parent, other->_bst_parent);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
WBTree<Node, NodeTraits, Options, Tag, Compare>::WBTree()
{}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
WBTree<Node, NodeTraits, Options, Tag, Compare>::WBTree(MyClass && other)
{
	this->root = other.root;
	other.root = nullptr;
	this->s = other.s;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <bool on_equality_prefer_left>
void
WBTree<Node, NodeTraits, Options, Tag, Compare>::insert_leaf_base(Node & node,
                                                                  Node * start)
{
	// TODO during fixup, we traverse the path to the root anyways. Update size in
	// that traverse!

	node.NB::set_right(nullptr);
	node.NB::set_left(nullptr);
	node.NB::_wbt_size = 1;

	Node * parent = start;
	Node * cur = start;

	while (cur != nullptr) {
		parent = cur;
		parent->NB::_wbt_size += 1;

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
	}

	if (parent == nullptr) {
		// new root!
		node.NB::set_parent(nullptr);
		this->root = &node;
		NodeTraits::leaf_inserted(node, *this);
	} else {
		node.NB::set_parent(parent);

		if (this->cmp(node, *parent)) {
			parent->NB::set_left(&node);
		} else if (this->cmp(*parent, node)) {
			parent->NB::set_right(&node);
		} else {
			// assert(multiple);
			if constexpr (!Options::multiple) {
				return;
			}

			if constexpr (on_equality_prefer_left) {
				parent->NB::set_left(&node);
			} else {
				parent->NB::set_right(&node);
			}
		}

		NodeTraits::leaf_inserted(node, *this);
		this->fixup_after_insert(&node);
	}

	return;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
WBTree<Node, NodeTraits, Options, Tag, Compare>::rotate_left(Node * parent)
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
WBTree<Node, NodeTraits, Options, Tag, Compare>::rotate_right(Node * parent)
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
WBTree<Node, NodeTraits, Options, Tag, Compare>::fixup_after_insert(Node * node)
{
	if (node->NB::get_parent() == nullptr) {
		return;
	}

	bool ascended_right = node->NB::get_parent()->NB::get_right() == node;
	node = node->NB::get_parent();
	size_t last_size = 1;
	size_t last_left = 0;
	size_t last_right = 0;

	while (node != nullptr) {
		if (ascended_right) {
			size_t left_weight =
			    node->NB::_wbt_size - 1 - last_size; // TODO this can be reformulated
			// TODO this is wrong6k66
			if ((left_weight + 1) * Options::wbt_delta() < (last_size + 1)) {
				// Out of balance with right-overhang

				if (last_left >= Options::wbt_gamma() * last_right) {
					// the right-left subtree is heavy enough to just take it
					// double rotation!

					// TODO FIXME this is quick&dirty - properly implement double
					// rotation!
					this->rotate_right(node->NB::get_right());
					this->rotate_left(node);

					// Rebuild the size of the rotated-down node
					node->NB::_wbt_size = 1;
					if (__builtin_expect(node->NB::get_right() != nullptr, true)) {
						node->NB::_wbt_size += node->NB::get_right()->NB::_wbt_size;
					}
					if (node->NB::get_left() != nullptr) {
						node->NB::_wbt_size += node->NB::get_left()->NB::_wbt_size;
					}

					Node * sibling = node->NB::get_parent()->NB::get_right();
					sibling->NB::_wbt_size = 1;
					if (__builtin_expect(sibling->NB::get_left() != nullptr, true)) {
						sibling->NB::_wbt_size += sibling->NB::get_left()->NB::_wbt_size;
					}
					if (sibling->NB::get_right() != nullptr) {
						sibling->NB::_wbt_size += sibling->NB::get_right()->NB::_wbt_size;
					}

					// switch to new parent
					node = node->NB::get_parent();
					// rebuilt its size
					node->NB::_wbt_size =
					    1 + sibling->NB::_wbt_size + node->NB::get_left()->NB::_wbt_size;

				} else {
					// Take the whole right subtree - single rotation

					this->rotate_left(node);

					// Rebuild the size of the rotated-down node
					node->NB::_wbt_size = 1;
					if (__builtin_expect(node->NB::get_right() != nullptr, true)) {
						node->NB::_wbt_size += node->NB::get_right()->NB::_wbt_size;
					}
					if (node->NB::get_left() != nullptr) {
						node->NB::_wbt_size += node->NB::get_left()->NB::_wbt_size;
					}

					// switch to new parent
					node = node->NB::get_parent();
					// rebuilt its size
					node->NB::_wbt_size = 1 + node->NB::get_left()->NB::_wbt_size;
					if (node->NB::get_right() != nullptr) {
						node->NB::_wbt_size += node->NB::get_right()->NB::_wbt_size;
					}
				}
			}

		} else {
			// Ascended left

			size_t right_weight =
			    node->NB::_wbt_size - 1 - last_size; // TODO this can be reformulated

			if ((right_weight + 1) * Options::wbt_delta() < (last_size + 1)) {
				// Out of balance with left-overhang

				if (last_right >= Options::wbt_gamma() * last_left) {
					// left-right subtree is large enough to only take that subtree -
					// double rotation!
					// TODO FIXME this is quick&dirty - properly implement double
					// rotation!
					this->rotate_left(node->NB::get_left());
					this->rotate_right(node);

					// Rebuild the size of the rotated-down node
					node->NB::_wbt_size = 1;
					if (__builtin_expect(node->NB::get_left() != nullptr, true)) {
						node->NB::_wbt_size += node->NB::get_left()->NB::_wbt_size;
					}
					if (node->NB::get_right() != nullptr) {
						node->NB::_wbt_size += node->NB::get_right()->NB::_wbt_size;
					}

					Node * sibling = node->NB::get_parent()->NB::get_left();
					sibling->NB::_wbt_size = 1;
					if (__builtin_expect(sibling->NB::get_right() != nullptr, true)) {
						sibling->NB::_wbt_size += sibling->NB::get_right()->NB::_wbt_size;
					}
					if (sibling->NB::get_left() != nullptr) {
						sibling->NB::_wbt_size += sibling->NB::get_left()->NB::_wbt_size;
					}

					// switch to new parent
					node = node->NB::get_parent();
					// rebuilt its size
					node->NB::_wbt_size =
					    1 + sibling->NB::_wbt_size + node->NB::get_right()->NB::_wbt_size;

				} else {
					// Take the whole left subtree
					this->rotate_right(node);

					// Rebuild the size of the rotated-down node
					node->NB::_wbt_size = 1;
					if (__builtin_expect(node->NB::get_left() != nullptr, true)) {
						node->NB::_wbt_size += node->NB::get_left()->NB::_wbt_size;
					}
					if (node->NB::get_right() != nullptr) {
						node->NB::_wbt_size += node->NB::get_right()->NB::_wbt_size;
					}

					// switch to new parent
					node = node->NB::get_parent();
					// rebuilt its size
					node->NB::_wbt_size = 1 + node->NB::get_right()->NB::_wbt_size;
					if (node->NB::get_left() != nullptr) {
						node->NB::_wbt_size += node->NB::get_left()->NB::_wbt_size;
					}
				}
			}
		} // end of "ascended left"

		// Ascend to the next!
		Node * old_node = node;
		last_size = node->NB::_wbt_size;
		if (__builtin_expect(node->NB::get_left() != nullptr,
		                     true)) { // TODO this is clear if we ascended…
			last_left = node->NB::get_left()->NB::_wbt_size;
		} else {
			last_left = 0;
		}
		if (__builtin_expect(node->NB::get_right() != nullptr,
		                     true)) { // TODO this is clear if we ascended…
			last_right = node->NB::get_right()->NB::_wbt_size;
		} else {
			last_right = 0;
		}

		node = node->NB::get_parent();
		if (node != nullptr) {
			ascended_right =
			    node->NB::get_right() == old_node; // TODO do this at the loop start
		}
	}
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
WBTree<Node, NodeTraits, Options, Tag, Compare>::insert(Node & node)
{
	this->s.add(1);
	this->insert_leaf_base<true>(node, this->root);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
WBTree<Node, NodeTraits, Options, Tag, Compare>::insert_left_leaning(
    Node & node)
{
	this->s.add(1);
	this->insert_leaf_base<true>(node, this->root);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
WBTree<Node, NodeTraits, Options, Tag, Compare>::insert_right_leaning(
    Node & node)
{
	this->s.add(1);
	this->insert_leaf_base<false>(node, this->root);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
WBTree<Node, NodeTraits, Options, Tag, Compare>::verify_sizes() const
{
	for (auto & node : *this) {
		size_t size = 1;
		if (node.NB::get_left() != nullptr) {
			size += node.NB::get_left()->NB::_wbt_size;
		}
		if (node.NB::get_right() != nullptr) {
			size += node.NB::get_right()->NB::_wbt_size;
		}

		debug::yggassert(size == node.NB::_wbt_size);
	}
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
WBTree<Node, NodeTraits, Options, Tag, Compare>::dbg_verify() const
{
	this->verify_tree();
	this->verify_order();
	this->verify_sizes();
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
bool
WBTree<Node, NodeTraits, Options, Tag, Compare>::verify_integrity() const
{
	try {
		this->dbg_verify();
	} catch (debug::VerifyException & e) {
		return false;
	}

	return true;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
WBTree<Node, NodeTraits, Options, Tag, Compare>::swap_nodes(Node * n1,
                                                            Node * n2)
{
	if (n1->NB::get_parent() == n2) {
		this->swap_neighbors(n2, n1);
	} else if (n2->NB::get_parent() == n1) {
		this->swap_neighbors(n1, n2);
	} else {
		this->swap_unrelated_nodes(n1, n2);
	}

	std::swap(n1->NB::_wbt_size, n2->NB::_wbt_size);

	NodeTraits::swapped(*n1, *n2, *this);
}

/*
template <class Node, class NodeTraits, class Options, class Tag, class
Compare> void WBTree<Node, NodeTraits, Options, Tag, Compare>::replace_node(
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
*/

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
WBTree<Node, NodeTraits, Options, Tag, Compare>::swap_neighbors(Node * parent,
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
WBTree<Node, NodeTraits, Options, Tag, Compare>::swap_unrelated_nodes(Node * n1,
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
WBTree<Node, NodeTraits, Options, Tag, Compare>::remove_to_leaf(Node & node)
{
	Node * cur = &node;
	Node * parent;
	bool deleted_from_right;

	// Size reduction is done during up-traversal!

	// TODO make configurable whether right-leaning or left-leaning?
	// or use size? (currently implemented)
	if ((cur->NB::get_left() != nullptr) &&
	    ((cur->NB::get_right() == nullptr) ||
	     (cur->NB::get_left()->NB::_wbt_size <=
	      cur->NB::get_right()->NB::_wbt_size))) {
		// Use the largest node on the left
		cur = cur->NB::get_left();
		while (cur->NB::get_right() != nullptr) {
			cur = cur->NB::get_right();
		}

		if (cur != &node) {
			this->swap_nodes(&node, cur);
		}

		// Now, node is where cur should point to
		cur = &node;
		parent = cur->NB::get_parent();
		NodeTraits::delete_leaf(*cur, *this);

		if (cur->NB::get_left() != nullptr) {
			// Update child's parent if there is such a child
			cur->NB::get_left()->NB::set_parent(parent);
		}

		if (__builtin_expect(parent == nullptr, false)) {
			this->root = cur->NB::get_left();
			deleted_from_right = true; // doesn't make a difference
		} else {
			// TODO this is only dependent on the number of steps we took in the
			// loop above!
			if (__builtin_expect(parent->NB::get_right() == cur, true)) {
				parent->NB::set_right(cur->NB::get_left());
				deleted_from_right = true;
			} else {
				parent->NB::set_left(cur->NB::get_left());
				deleted_from_right = false;
			}
			NodeTraits::deleted_below(*parent, *this);
		}

		this->fixup_after_delete(parent, deleted_from_right);

	} else if (cur->NB::get_right() != nullptr) {
		// use the smallest on the right
		cur = cur->NB::get_right();
		while (cur->NB::get_left() != nullptr) {
			cur = cur->NB::get_left();
		}

		if (cur != &node) {
			this->swap_nodes(&node, cur);
		}

		// Now, node is where cur should point to
		cur = &node;
		parent = cur->NB::get_parent();
		NodeTraits::delete_leaf(*cur, *this);

		if (cur->NB::get_right() != nullptr) {
			// Update child's parent if there is such a child
			cur->NB::get_right()->NB::set_parent(parent);
		}

		if (__builtin_expect(parent == nullptr, false)) {
			this->root = cur->NB::get_right();
			deleted_from_right = true; // doesn't make a difference
		} else {
			// TODO this is only dependent on the number of steps we took in the
			// loop above!
			if (__builtin_expect(parent->NB::get_left() == cur, true)) {
				parent->NB::set_left(cur->NB::get_right());
				deleted_from_right = false;
			} else {
				parent->NB::set_right(cur->NB::get_right());
				deleted_from_right = true;
			}
			NodeTraits::deleted_below(*parent, *this);
		}

		this->fixup_after_delete(parent, deleted_from_right);
	} else {
		// This is a leaf!
		parent = cur->NB::get_parent();
		NodeTraits::delete_leaf(*cur, *this);

		if (__builtin_expect(parent == nullptr, false)) {
			this->root = nullptr;
			deleted_from_right = true; // doesn't make a difference
		} else {
			if (__builtin_expect(parent->NB::get_left() == cur, true)) {
				parent->NB::set_left(nullptr);
				deleted_from_right = false;
			} else {
				parent->NB::set_right(nullptr);
				deleted_from_right = true;
			}
			NodeTraits::deleted_below(*parent, *this);
		}
		this->fixup_after_delete(parent, deleted_from_right);
	}
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
WBTree<Node, NodeTraits, Options, Tag, Compare>::fixup_after_delete(
    Node * parent, bool deleted_right)
{
	if (parent == nullptr) { // TODO can this happen?
		return;
	}

	bool ascended_right = deleted_right;
	size_t last_size = 0;

	if (deleted_right) {
		if (parent->NB::get_right() != nullptr) {
			last_size = parent->NB::get_right()->NB::_wbt_size;
		}
	} else {
		if (parent->NB::get_left() != nullptr) {
			last_size = parent->NB::get_left()->NB::_wbt_size;
		}
	}

	Node * node = parent;

	// TODO unify size -> weight *everywhere*
	while (node != nullptr) {
		node->NB::_wbt_size -= 1;

		if (!ascended_right) {
			// ascended left!

			size_t right_weight =
			    node->NB::_wbt_size - 1 - last_size; // TODO this can be reformulated

			if ((last_size + 1) * Options::wbt_delta() < (right_weight + 1)) {
				// Out of balance with right-overhang

				size_t right_left = 0;
				size_t right_right = 0;

				// There must be a child on the right.
				if (node->NB::get_right()->NB::get_left() != nullptr) {
					right_left = node->NB::get_right()->NB::get_left()->NB::_wbt_size;
				}
				if (node->NB::get_right()->NB::get_right() != nullptr) {
					right_right = node->NB::get_right()->NB::get_right()->NB::_wbt_size;
				}

				if (right_left >= Options::wbt_gamma() * right_right) {
					// the right-left subtree is heavy enough to just take it
					// double rotation!

					// TODO FIXME this is quick&dirty - properly implement double
					// rotation!
					this->rotate_right(node->NB::get_right());
					this->rotate_left(node);

					// Rebuild the size of the rotated-down node
					node->NB::_wbt_size = 1;
					if (__builtin_expect(node->NB::get_right() != nullptr, true)) {
						node->NB::_wbt_size += node->NB::get_right()->NB::_wbt_size;
					}
					if (node->NB::get_left() != nullptr) {
						node->NB::_wbt_size += node->NB::get_left()->NB::_wbt_size;
					}

					Node * sibling = node->NB::get_parent()->NB::get_right();
					sibling->NB::_wbt_size = 1;
					if (__builtin_expect(sibling->NB::get_left() != nullptr, true)) {
						sibling->NB::_wbt_size += sibling->NB::get_left()->NB::_wbt_size;
					}
					if (sibling->NB::get_right() != nullptr) {
						sibling->NB::_wbt_size += sibling->NB::get_right()->NB::_wbt_size;
					}

					// switch to new parent
					node = node->NB::get_parent();
					// rebuilt its size
					node->NB::_wbt_size =
					    1 + sibling->NB::_wbt_size + node->NB::get_left()->NB::_wbt_size;

				} else {
					// Take the whole right subtree - single rotation

					this->rotate_left(node);

					// Rebuild the size of the rotated-down node
					node->NB::_wbt_size = 1;
					if (__builtin_expect(node->NB::get_right() != nullptr, true)) {
						node->NB::_wbt_size += node->NB::get_right()->NB::_wbt_size;
					}
					if (node->NB::get_left() != nullptr) {
						node->NB::_wbt_size += node->NB::get_left()->NB::_wbt_size;
					}

					// switch to new parent
					node = node->NB::get_parent();
					// rebuilt its size
					node->NB::_wbt_size = 1 + node->NB::get_left()->NB::_wbt_size;
					if (node->NB::get_right() != nullptr) {
						node->NB::_wbt_size += node->NB::get_right()->NB::_wbt_size;
					}
				}
			}

		} else {
			// Ascended right

			size_t left_weight =
			    node->NB::_wbt_size - 1 - last_size; // TODO this can be reformulated

			if ((last_size + 1) * Options::wbt_delta() < (left_weight + 1)) {
				// Out of balance with left-overhang

				size_t left_left = 0;
				size_t left_right = 0;

				// There must be a child on the left.
				if (node->NB::get_left()->NB::get_left() != nullptr) {
					left_left = node->NB::get_left()->NB::get_left()->NB::_wbt_size;
				}
				if (node->NB::get_left()->NB::get_right() != nullptr) {
					left_right = node->NB::get_left()->NB::get_right()->NB::_wbt_size;
				}

				if (left_right >= Options::wbt_gamma() * left_left) {
					// left-right subtree is large enough to only take that subtree -
					// double rotation!
					// TODO FIXME this is quick&dirty - properly implement double
					// rotation!
					this->rotate_left(node->NB::get_left());
					this->rotate_right(node);

					// Rebuild the size of the rotated-down node
					node->NB::_wbt_size = 1;
					if (__builtin_expect(node->NB::get_left() != nullptr, true)) {
						node->NB::_wbt_size += node->NB::get_left()->NB::_wbt_size;
					}
					if (node->NB::get_right() != nullptr) {
						node->NB::_wbt_size += node->NB::get_right()->NB::_wbt_size;
					}

					Node * sibling = node->NB::get_parent()->NB::get_left();
					sibling->NB::_wbt_size = 1;
					if (__builtin_expect(sibling->NB::get_right() != nullptr, true)) {
						sibling->NB::_wbt_size += sibling->NB::get_right()->NB::_wbt_size;
					}
					if (sibling->NB::get_left() != nullptr) {
						sibling->NB::_wbt_size += sibling->NB::get_left()->NB::_wbt_size;
					}

					// switch to new parent
					node = node->NB::get_parent();
					// rebuilt its size
					node->NB::_wbt_size =
					    1 + sibling->NB::_wbt_size + node->NB::get_right()->NB::_wbt_size;

				} else {
					// Take the whole left subtree
					this->rotate_right(node);

					// Rebuild the size of the rotated-down node
					node->NB::_wbt_size = 1;
					if (__builtin_expect(node->NB::get_left() != nullptr, true)) {
						node->NB::_wbt_size += node->NB::get_left()->NB::_wbt_size;
					}
					if (node->NB::get_right() != nullptr) {
						node->NB::_wbt_size += node->NB::get_right()->NB::_wbt_size;
					}

					// switch to new parent
					node = node->NB::get_parent();
					// rebuilt its size
					node->NB::_wbt_size = 1 + node->NB::get_right()->NB::_wbt_size;
					if (node->NB::get_left() != nullptr) {
						node->NB::_wbt_size += node->NB::get_left()->NB::_wbt_size;
					}
				}
			}

		} // end of "ascended right"

		// Ascend to the next!
		Node * old_node = node;
		last_size = node->NB::_wbt_size;
		node = node->NB::get_parent();
		if (node != nullptr) {
			ascended_right =
			    node->NB::get_right() == old_node; // TODO do this at the loop start
		}
	}
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
WBTree<Node, NodeTraits, Options, Tag, Compare>::remove(Node & node)
{
	this->s.reduce(1);

	// TODO collapse this method
	this->remove_to_leaf(node);
}

} // namespace weight
} // namespace ygg

#endif // YGG_RBTREE_CPP
