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
WBTree<Node, NodeTraits, Options, Tag, Compare>::insert_leaf_onepass(
    Node & node)
{
	//	std::cout << "\n============= Inserting ============\n";

	node.NB::set_right(nullptr);
	node.NB::set_left(nullptr);
	node.NB::_wbt_size = 1;

	if (__builtin_expect(this->root == nullptr, false)) {
		// std::cout << "Root case.\n";
		this->root = &node;
		node.NB::set_parent(nullptr);
		NodeTraits::leaf_inserted(node, *this);
		return;
	}

	Node * cur = this->root;

	Node * parent;
	bool left_of_parent;

	Node * n_r;
	Node * n_rr;
	Node * n_rl;

	Node * n_l;
	Node * n_ll;
	Node * n_lr;

	Node * single_next;
	size_t single_cur_delta;

	// TODO pragma unroll n for gcc 8?
	while (cur != nullptr) {
		// TODO use knowledge from earlier cases?
		// std::cout << "We're at node " << std::hex << cur << std::dec << "\n";

		if (this->cmp(*cur, node)) { // Case R*
			// std::cout << "> Going Right\n";
			/* We will descend to the right. Check if this breaks our balance and if
			 * so, rotate. */
			n_r = cur->NB::get_right();
			if (__builtin_expect(n_r != nullptr, true)) {
				size_t r_size = n_r->NB::_wbt_size + 1;
				size_t l_size = cur->NB::_wbt_size - r_size - 1;

				if ((l_size + 1) * Options::wbt_delta() <
				    (r_size + 1)) { // TODO incorporate the plus/minus above
					// Right-overhang - we must rotate
					// std::cout << ">> Right-Overhang\n";
					size_t rr_size = 0;
					n_rr = n_r->NB::get_right();
					if (__builtin_expect(n_rr != nullptr, true)) {
						rr_size += n_rr->NB::_wbt_size;
					}
					size_t rl_size = 0;
					n_rl = n_r->NB::get_left();
					if (__builtin_expect(n_rl != nullptr, true)) {
						rl_size += n_rl->NB::_wbt_size;
					}

					// TODO flip this for on_equality_prefer_left
					bool case_rr = false;        // TODO unroll this?
					if (this->cmp(*n_r, node)) { // Case RR
						rr_size += 1;

						single_next = n_rr;
						single_cur_delta = 0;
						parent = n_r;

						case_rr = true;
					} else { // Case RL*
						rl_size += 1;

						single_next = n_rl;
						single_cur_delta = 1;
						parent = cur;
					}

					if ((rl_size + 1) >= Options::wbt_gamma() * (rr_size + 1)) {
						// the right-left subtree is heavy enough to just take it
						// double rotation!
						// std::cout << ">>> Double rotation.\n";

						// Special case: n_rl does not exist yet, but is the node to be
						// inserted (that is the only way it can be still empty and heavier
						// than an empty n_rr subtree) we handle this specially
						if (n_rl == nullptr) {
							/*
							std::cout << ">>>> Super-Special case!\n";
							std::cout << ">>>>> Node is " << std::hex << &node << " / cur is "
							          << cur << " / n_r is " << n_r << std::dec << "\n";
							          std::cout << "Cur size before is: " <<
							cur->NB::_wbt_size << "\n";*/
							node.NB::set_parent(cur->NB::get_parent());
							node.NB::set_right(n_r);
							node.NB::set_left(cur);
							node.NB::_wbt_size = cur->NB::_wbt_size + 1;

							if (__builtin_expect(cur->NB::get_parent() != nullptr, true)) {
								if (cur->NB::get_parent()->NB::get_left() == cur) {
									cur->NB::get_parent()->NB::set_left(&node);
								} else {
									cur->NB::get_parent()->NB::set_right(&node);
								}
							} else {
								this->root = &node;
							}
							cur->NB::set_right(nullptr);
							cur->NB::set_parent(&node);
							cur->NB::_wbt_size -= (r_size - 1);

							n_r->NB::set_parent(&node);

							NodeTraits::leaf_inserted(node, *this);
							return;
						}

						this->rotate_right(n_r);
						this->rotate_left(cur);

						n_rl->NB::_wbt_size += 1;

						if (case_rr) {
							// std::cout << ">>>> Case RR\n";
							n_r->NB::_wbt_size += 1;

							parent = n_r;
							left_of_parent = false;

							cur = n_rr;
						} else {
							// For the rl case, we need to distuinguish between rlr and rll
							// Note that in the case of a double rotation, n_rl *cannot* be
							// null

							if (this->cmp(*n_rl, node)) { // case rlr
								// std::cout << ">>>> Case RLR\n";
								n_r->NB::_wbt_size += 1;
								parent = n_r;
								left_of_parent = true;
								cur = n_r->NB::get_left(); // that's where rlr lives now
							} else {                     // case rll
								// std::cout << ">>>> Case RLL\n";
								cur->NB::_wbt_size += 1;
								parent = cur;
								left_of_parent = false;
								cur = cur->NB::get_right(); // that's where rll lives now
							}
						}

					} else {
						// Single rotation does the trick
						// std::cout << ">>> Single Rotation.\n";
						this->rotate_left(cur);

						n_r->NB::_wbt_size += 1;
						cur->NB::_wbt_size += single_cur_delta;

						// We determined above where to continue
						left_of_parent = false;
						cur = single_next;
					}
				} else {
					// std::cout << ">> No overhang.\n";
					// no rotation. Continue at r
					// Only the size of cur must be updated
					cur->NB::_wbt_size += 1;
					// In this case, n_r is not null, thus we continue and do not have to
					// set a parent
					cur = n_r;
				}
			} else {
				// we should descend right in cur, but there is no node. Adjust size,
				// set parent and break loop.
				// TODO can this break balance?
				// std::cout << "> Finished on the right!\n";

				cur->NB::_wbt_size += 1;
				parent = cur;
				left_of_parent = false;
				break;
			}
		} else { // Case L*
			// std::cout << "< Going Left\n";
			/* We will descend to the left. Check if this breaks our balance and if
			 * so, rotate. */
			n_l = cur->NB::get_left();

			if (__builtin_expect(n_l != nullptr, true)) {
				size_t l_size = n_l->NB::_wbt_size + 1;
				size_t r_size = cur->NB::_wbt_size - l_size - 1;

				if ((r_size + 1) * Options::wbt_delta() <
				    (l_size + 1)) { // TODO incorporate the plus/minus above
					// Left-overhang - we must rotate
					// std::cout << "<< Left-Overhang\n";

					size_t lr_size = 0;
					n_lr = n_l->NB::get_right();
					if (__builtin_expect(n_lr != nullptr, true)) { // TODO expectations?
						lr_size += n_lr->NB::_wbt_size;
					}
					size_t ll_size = 0;
					n_ll = n_l->NB::get_left();
					if (__builtin_expect(n_ll != nullptr, true)) {
						ll_size += n_ll->NB::_wbt_size;
					}

					// TODO flip this for on_equality_prefer_left
					bool case_ll = false;         // TODO unroll this?
					if (!this->cmp(*n_l, node)) { // Case LL
						ll_size += 1;

						single_next = n_ll;
						single_cur_delta = 0;
						parent = n_l;

						case_ll = true;
					} else { // Case LR*
						lr_size += 1;

						single_next = n_lr;
						single_cur_delta = 1;
						parent = cur;
					}

					if ((lr_size + 1) >= Options::wbt_gamma() * (ll_size + 1)) {
						// the left-right subtree is heavy enough to just take it
						// double rotation!
						// std::cout << "<<< Double Rotation!\n";
						// Special case: n_lr does not exist yet, but is the node to be
						// inserted (that is the only way it can be still empty and heavier
						// than an empty n_ll subtree) we handle this specially
						if (n_lr == nullptr) {
							// std::cout << "<<<< Super-special case!\n";
							node.NB::set_parent(cur->NB::get_parent());
							node.NB::set_left(n_l);
							node.NB::set_right(cur);
							node.NB::_wbt_size = cur->NB::_wbt_size + 1;

							if (__builtin_expect(cur->NB::get_parent() != nullptr, true)) {
								if (cur->NB::get_parent()->NB::get_left() == cur) {
									cur->NB::get_parent()->NB::set_left(&node);
								} else {
									cur->NB::get_parent()->NB::set_right(&node);
								}
							} else {
								this->root = &node;
							}

							cur->NB::set_left(nullptr);
							cur->NB::set_parent(&node);
							cur->NB::_wbt_size -= (l_size - 1);

							n_l->NB::set_parent(&node);

							NodeTraits::leaf_inserted(node, *this);
							return;
						}

						this->rotate_left(n_l);
						this->rotate_right(cur);

						n_lr->NB::_wbt_size += 1;

						if (case_ll) {
							// std::cout << "<<<< Case LL\n";
							n_l->NB::_wbt_size += 1;

							cur = n_ll;
							parent = n_l;
							left_of_parent = true;

						} else {
							// For the lr case, we need to distuinguish between lrl and lrr
							// Note that in the case of a double rotation, n_lr *cannot* be
							// null

							if (!this->cmp(*n_lr, node)) { // case lrl
								// std::cout << "<<<< Case LRL\n";
								n_l->NB::_wbt_size += 1;
								parent = n_l;
								left_of_parent = false;
								cur = n_l->NB::get_right(); // that's where lrl lives now
							} else {                      // case lrr
								// std::cout << "<<<< Case LRR\n";

								cur->NB::_wbt_size += 1;
								parent = cur;
								left_of_parent = true;
								cur = cur->NB::get_left(); // that's where lrr lives now
							}
						}

					} else {
						// std::cout << "<<< Single Rotation\n";
						// Single rotation does the trick
						this->rotate_right(cur);

						n_l->NB::_wbt_size += 1;
						cur->NB::_wbt_size += single_cur_delta;

						// We determined above where to continue
						left_of_parent = true;
						cur = single_next;
					}
				} else {
					// std::cout << "<< No overhang.\n";
					// no rotation. Continue at l
					// Only the size of cur must be updated
					cur->NB::_wbt_size += 1;
					// in this case, n_l is not null, thus we continue and do not need to
					// set a parent
					cur = n_l;
				}
			} else {
				// std::cout << "< Finished on the left!\n";
				// we should descend left, but there is no n_l. Adjust size, set parent
				// and break loop.
				// TODO can this break balance?
				cur->NB::_wbt_size += 1;
				parent = cur;
				left_of_parent = true;
				break;
			}
		}
	}

	// After the loop, we are able to just insert below parent
	// std::cout << "Inserting " << std::hex << &node << " below " << parent <<
	// std::dec << "\n";
	node.NB::set_parent(parent);
	if (left_of_parent) {
		parent->NB::set_left(&node);
	} else {
		parent->NB::set_right(&node);
	}
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <bool on_equality_prefer_left>
void
WBTree<Node, NodeTraits, Options, Tag, Compare>::insert_leaf_base_twopass(
    Node & node, Node * start)
{
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
		this->fixup_after_insert_twopass(&node);
	}

	return;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
WBTree<Node, NodeTraits, Options, Tag, Compare>::rotate_left(Node * parent)
{
	Node * right_child = parent->NB::get_right();

	size_t right_child_old_size = right_child->NB::_wbt_size;
	right_child->NB::_wbt_size = parent->NB::_wbt_size;
	parent->NB::_wbt_size -= right_child_old_size;

	parent->NB::set_right(right_child->NB::get_left());
	if (right_child->NB::get_left() != nullptr) {
		right_child->NB::get_left()->NB::set_parent(parent);

		parent->NB::_wbt_size += right_child->NB::get_left()->NB::_wbt_size;
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

	size_t left_child_old_size = left_child->NB::_wbt_size;
	left_child->NB::_wbt_size = parent->NB::_wbt_size;
	parent->NB::_wbt_size -= left_child_old_size;

	parent->NB::set_left(left_child->NB::get_right());
	if (left_child->NB::get_right() != nullptr) {
		left_child->NB::get_right()->NB::set_parent(parent);

		parent->NB::_wbt_size += left_child->NB::get_right()->NB::_wbt_size;
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

// TODO do the fixup already when running down!
template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
WBTree<Node, NodeTraits, Options, Tag, Compare>::fixup_after_insert_twopass(
    Node * node)
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

					// switch to new parent
					node = node->NB::get_parent();
				} else {
					// Take the whole right subtree - single rotation
					this->rotate_left(node);

					// switch to new parent
					node = node->NB::get_parent();
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

					// switch to new parent
					node = node->NB::get_parent();
				} else {
					// Take the whole left subtree
					this->rotate_right(node);

					// switch to new parent
					node = node->NB::get_parent();
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
	if constexpr (Options::wbt_single_pass) {
		this->insert_leaf_onepass<true>(node);
	} else {
		this->insert_leaf_base_twopass<true>(node, this->root);
	}
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
WBTree<Node, NodeTraits, Options, Tag, Compare>::insert_left_leaning(
    Node & node)
{
	this->s.add(1);
	this->insert_leaf_base_twopass<true>(node, this->root);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
WBTree<Node, NodeTraits, Options, Tag, Compare>::insert_right_leaning(
    Node & node)
{
	this->s.add(1);
	this->insert_leaf_base_twopass<false>(node, this->root);
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

					// switch to new parent
					node = node->NB::get_parent();
				} else {
					// Take the whole right subtree - single rotation
					this->rotate_left(node);

					// switch to new parent
					node = node->NB::get_parent();
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

					// switch to new parent
					node = node->NB::get_parent();
				} else {
					// Take the whole left subtree
					this->rotate_right(node);

					// switch to new parent
					node = node->NB::get_parent();
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
