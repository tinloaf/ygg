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
						// than an empty n_rr subtree) we handle this specially.
						// We insert the node first, then do the double rotation, and are
						// done.
						if (n_rl == nullptr) {
							node.NB::set_parent(n_r);
							n_r->NB::set_left(&node);
							n_r->NB::_wbt_size += 1;
							cur->NB::_wbt_size += 1;

							NodeTraits::leaf_inserted(node, *this);

							this->rotate_right(n_r);
							this->rotate_left(cur);

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

					if ((lr_size + 1) > Options::wbt_gamma() * (ll_size + 1)) {
						// the left-right subtree is heavy enough to just take it
						// double rotation!
						// std::cout << "<<< Double Rotation!\n";

						// Special case: n_lr does not exist yet, but is the node to be
						// inserted (that is the only way it can be still empty and heavier
						// than an empty n_ll subtree) we handle this specially.
						// We insert the node first, then do the double rotation, and are
						// done.
						if (n_lr == nullptr) {
							// std::cout << "<<<< Super-special case!\n";
							node.NB::set_parent(n_l);
							n_l->NB::set_right(&node);
							n_l->NB::_wbt_size += 1;
							cur->NB::_wbt_size += 1;

							NodeTraits::leaf_inserted(node, *this);

							this->rotate_left(n_l);
							this->rotate_right(cur);

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
	NodeTraits::leaf_inserted(node, *this);
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
			if ((typename Options::WBTDeltaT)((left_weight + 1) *
			                                  Options::wbt_delta()) <
			    (typename Options::WBTDeltaT)(last_size + 1)) {
				// Out of balance with right-overhang

				if ((typename Options::WBTGammaT)last_left >
				    (typename Options::WBTGammaT)(Options::wbt_gamma() * last_right)) {
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

			if ((typename Options::WBTDeltaT)((right_weight + 1) *
			                                  Options::wbt_delta()) <
			    (typename Options::WBTDeltaT)(last_size + 1)) {
				// Out of balance with left-overhang

				if ((typename Options::WBTGammaT)last_right >
				    (typename Options::WBTGammaT)(Options::wbt_gamma() * last_left)) {
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
size_t
WBTree<Node, NodeTraits, Options, Tag, Compare>::dbg_count_violations() const
{
	size_t result = 0;
	for (const Node & n : *this) {
		size_t left = 0;
		size_t right = 0;
		if (n.NB::get_left() != nullptr) {
			left = n.NB::get_left()->NB::_wbt_size;
		}
		if (n.NB::get_right() != nullptr) {
			right = n.NB::get_right()->NB::_wbt_size;
		}

		if (((typename Options::WBTDeltaT)(left * Options::wbt_delta()) <
		     (typename Options::WBTDeltaT)(right)) ||
		    ((typename Options::WBTDeltaT)(right * Options::wbt_delta()) <
		     (typename Options::WBTDeltaT)(left))) {
			result += 1;
		}
	}

	return result;
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
	if (n1->NB::get_parent() ==
	    n2) { // TODO this should never happen, since n2 is always the descendant
		this->swap_neighbors(n2, n1);
	} else if (n2->NB::get_parent() == n1) {
		// std::cout << " ## Swapping neighbors 2.\n";
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
	// std::cout << "Swap neighbors!\n";
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
	// std::cout << "Swap unrelated!\n";

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
template <class Comparable>
bool
WBTree<Node, NodeTraits, Options, Tag, Compare>::erase(const Comparable & c)
{
	auto el = this->find(c);
	if (el != this->end()) {
		this->remove(*el);
		return true;
	} else {
		return false;
	}
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <class Comparable>
void
WBTree<Node, NodeTraits, Options, Tag, Compare>::erase_optimistic(
    const Comparable & c)
{
	Node * cur = this->root;

	size_t s_cur = cur->NB::_wbt_size - 1;

	while (true) {
		// std::cout << "## Now at " << std::hex << cur << std::dec << "\n";
		if (this->cmp(*cur, c)) {
			// descend right
			cur->NB::_wbt_size -= 1;
			Node * n_r = cur->NB::get_right(); // Since we're optimistic, we know that
			                                   // n_r is not nullptr
			size_t s_r = n_r->NB::_wbt_size - 1;

			// Step 1: Check balance
			if ((typename Options::WBTDeltaT)(s_r * Options::wbt_delta()) <
			    (typename Options::WBTDeltaT)(s_cur - s_r - 1)) {
				// std::cout << " ### Left-overhang \n";
				// Out of balance with left-overhang
				size_t s_lr = 0;
				size_t s_ll = 0;
				Node * n_l = cur->NB::get_left();
				Node * n_ll = n_l->NB::get_left();
				Node * n_lr = n_l->NB::get_right();

				if (n_lr != nullptr) {
					s_lr += n_lr->_wbt_size;
				}
				if (n_ll != nullptr) {
					s_ll += n_ll->_wbt_size;
				}

				if ((typename Options::WBTGammaT)s_lr >
				    ((typename Options::WBTGammaT)Options::wbt_gamma() * s_ll)) {
					// Double rotation
					// std::cout << " #### Double rotation \n";
					this->rotate_left(n_l);
					this->rotate_right(cur);
				} else {
					// std::cout << " #### Single rotation \n";
					this->rotate_right(cur);
				}
			}

			// Step 2: Actually go right
			cur = n_r;
			s_cur = s_r;
		} else if (this->cmp(c, *cur)) {
			// descend left
			cur->NB::_wbt_size -= 1;
			Node * n_l = cur->NB::get_left(); // Since we're optimistic, we know that
			                                  // n_l is not nullptr
			size_t s_l = n_l->NB::_wbt_size - 1;

			// Step 1: Check balance
			if (s_l * Options::wbt_delta() < (s_cur - s_l - 1)) {
				// Out of balance with right-overhang
				// std::cout << " ### Right overhang\n";
				size_t s_rr = 0;
				size_t s_rl = 0;
				Node * n_r = cur->NB::get_right();
				Node * n_rl = n_r->NB::get_left();
				Node * n_rr = n_r->NB::get_right();

				if (n_rr != nullptr) {
					s_rr += n_rr->_wbt_size;
				}
				if (n_rl != nullptr) {
					s_rl += n_rl->_wbt_size;
				}

				if (s_rl > Options::wbt_gamma() * s_rr) {
					// std::cout << " #### double rotation\n";
					// Double rotation
					this->rotate_right(n_r);
					this->rotate_left(cur);
				} else {
					// std::cout << " #### single rotation\n";
					this->rotate_left(cur);
				}
			}

			// Step 2: Actually go left
			cur = n_l;
			s_cur = s_l;
		} else {
			// std::cout << " ### Found!\n";
			// Element found - delete it!
			// std::cout << "Calling remove_onepass at " << std::hex << cur <<
			// std::dec
			// << "\n";
			this->remove_onepass<false>(*cur);
			return;
		}
	}
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <bool fix_upward>
void
WBTree<Node, NodeTraits, Options, Tag, Compare>::remove_onepass(Node & node)
{
	/* Basic idea: perform fixup for the part below node as we go down. Then fix
	 * upwards of node.
	 */

	Node * cur = &node;
	size_t s_cur;

	// Stores the parent of the subtree within which we delete. Everything above
	// it must be fixed upwards. Everything within it will be fixed inside this
	// method.
	Node * subtree_parent;
	bool subtree_parent_right = false;
	if constexpr (fix_upward) {
		subtree_parent = node.NB::get_parent();
		if (subtree_parent != nullptr) {
			subtree_parent_right = (subtree_parent->NB::get_right() == &node);
		}
	}

	if ((cur->NB::get_left() != nullptr) &&
	    ((cur->NB::get_right() == nullptr) ||
	     (cur->NB::get_left()->NB::_wbt_size >
	      cur->NB::get_right()->NB::_wbt_size))) {
		// Use the largest node on the left
		// std::cout << "-- Deleting from left.\n";

		Node * n_l = cur->NB::get_left();
		size_t s_left = n_l->NB::_wbt_size - 1; // deletion occurrs here
		cur->NB::_wbt_size -= 1;
		s_cur = cur->NB::_wbt_size;

		if (s_left * Options::wbt_delta() < (s_cur - s_left - 1)) {
			// Out of balance with right overhang
			// std::cout << " --- Initial right overhang\n";
			size_t s_rl = 0;
			size_t s_rr = 0;
			Node * n_r = cur->NB::get_right(); // n_r can not be nullptr, since the
			                                   // right subtree is heavy
			Node * n_rr = n_r->NB::get_right();
			Node * n_rl = n_r->NB::get_left();

			if (n_rr != nullptr) {
				s_rr += n_rr->NB::_wbt_size;
			}
			if (n_rl != nullptr) {
				s_rl += n_rl->NB::_wbt_size;
			}

			if (s_rl > Options::wbt_gamma() * s_rr) {
				// Double rotation
				// std::cout << " ---- Double rotation around " << std::hex << n_r
				// << " and " << cur << std::dec << "\n";
				this->rotate_right(n_r);
				this->rotate_left(cur);

			} else {
				// Single rotation
				// std::cout << " ---- Single rotation around " << std::hex << cur
				// << std::dec << "\n";
				this->rotate_left(cur);
			}
		}

		// We descend to the left, as decided ealier.
		// TODO FIXME decrement the size here or later during fixup?
		cur = n_l;
		s_cur = s_left;

		while (cur->NB::get_right() != nullptr) {
			/* Now, we keep descending right, fixing imbalances as we go
			 */
			// std::cout << " --- Now descending at " << std::hex << cur << std::dec
			// << "\n";
			cur->NB::_wbt_size -= 1;

			// Step 1: Check if descending right will hurt balance
			Node * n_r = cur->NB::get_right();
			size_t s_r = n_r->NB::_wbt_size - 1; // this is where deletion happens
			if (s_r * Options::wbt_delta() < (s_cur - s_r - 1)) {
				// Out of balance with left overhang

				size_t s_lr = 0;
				size_t s_ll = 0;
				n_l = cur->NB::get_left();
				Node * n_ll = n_l->NB::get_left();
				Node * n_lr = n_l->NB::get_right();

				if (n_lr != nullptr) {
					s_lr += n_lr->_wbt_size;
				}
				if (n_ll != nullptr) {
					s_ll += n_ll->_wbt_size;
				}

				if (s_lr > Options::wbt_gamma() * s_ll) {
					// Double rotation
					this->rotate_left(n_l);
					this->rotate_right(cur);
				} else {
					this->rotate_right(cur);
				}
			}

			// Step 2: Actually go right
			s_cur = s_r;
			cur = n_r;
		}

		this->remove_swap_and_remove_left<false>(&node, cur);
		if constexpr (fix_upward) {
			this->fixup_after_delete(subtree_parent, subtree_parent_right);
		}
	} else if (cur->NB::get_right() != nullptr) {
		// use the smallest on the right
		// std::cout << "-- Deleting from right.\n";

		Node * n_r = cur->NB::get_right();
		size_t s_right = n_r->NB::_wbt_size - 1; // deletion occurrs here
		cur->NB::_wbt_size -= 1;
		s_cur = cur->NB::_wbt_size;

		if (s_right * Options::wbt_delta() <
		    (s_cur - s_right - 1)) { // TODO FIXME has s_cur already the -1? (also
			                           // in other branch)
			// std::cout << " --- Initial left overhang\n";
			// Out of balance with left overhang
			size_t s_ll = 0;
			size_t s_lr = 0;
			Node * n_l = cur->NB::get_left(); // n_l can not be nullptr, since the
			                                  // left subtree is heavy
			Node * n_lr = n_l->NB::get_right();
			Node * n_ll = n_l->NB::get_left();

			if (n_lr != nullptr) {
				s_lr += n_lr->NB::_wbt_size;
			}
			if (n_ll != nullptr) {
				s_ll += n_ll->NB::_wbt_size;
			}

			if (s_lr > Options::wbt_gamma() * s_ll) {
				// Double rotation
				// std::cout << " ---- Double rotation.\n";
				this->rotate_left(n_l);
				this->rotate_right(cur);

			} else {
				// Sinlge Rotation
				// std::cout << " ---- Single rotation around " << std::hex << cur
				// << std::dec << ".\n";
				this->rotate_right(cur);
			}
		}

		// We descend to the right, as decided ealier.
		// TODO FIXME decrement the size here or later during fixup?
		cur = n_r;
		s_cur = s_right;

		while (cur->NB::get_left() != nullptr) {
			/* Now, we keep descending left, fixing imbalances as we go
			 */

			cur->NB::_wbt_size -= 1;
			// std::cout << " --- Now descending at " << std::hex << cur << std::dec
			// << "\n";
			// Step 1: Check if descending left will hurt balance
			Node * n_l = cur->NB::get_left();
			size_t s_l = n_l->NB::_wbt_size - 1; // this is where deletion happens
			if (s_l * Options::wbt_delta() <
			    (s_cur - s_l - 1)) { // TODO FIXME here the -1 is also missing!
				// Out of balance with right overhang
				// std::cout << " ---- Right overhang\n";
				size_t s_rr = 0;
				size_t s_rl = 0;
				n_r = cur->NB::get_right();
				Node * n_rl = n_r->NB::get_left();
				Node * n_rr = n_r->NB::get_right();

				if (n_rr != nullptr) {
					s_rr += n_rr->_wbt_size;
				}
				if (n_rl != nullptr) {
					s_rl += n_rl->_wbt_size;
				}

				if (s_rl > Options::wbt_gamma() * s_rr) {
					// Double rotation
					// std::cout << " ----- Double Rotation around " << std::hex << n_r <<
					// " and " << cur << std::dec << "\n";
					this->rotate_right(n_r);
					this->rotate_left(cur);
				} else {
					// std::cout << " ----- Single Rotation around " << std::hex << cur <<
					// "\n";
					this->rotate_left(cur);
				}
			}

			// Step 2: Actually go left
			s_cur = s_l;
			cur = n_l;
		}

		this->remove_swap_and_remove_right<false>(&node, cur);
		// cur is now where the node originally was. We need to fixup from there to
		// the root!
		if constexpr (fix_upward) {
			this->fixup_after_delete(subtree_parent, subtree_parent_right);
		}

	} else {
		// std::cout << " --- Deleting leaf!\n";
		// This is a leaf!
		this->remove_leaf<fix_upward>(&node);
	}
}

// TODO every sibling-less leaf currently leads to an overhang

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <bool call_fixup>
void
WBTree<Node, NodeTraits, Options, Tag, Compare>::remove_leaf(Node * node)
{
	Node * parent = node->NB::get_parent();
	NodeTraits::delete_leaf(*node, *this);
	bool deleted_from_right;

	if (__builtin_expect(parent == nullptr, false)) {
		this->root = nullptr;
		deleted_from_right = true; // doesn't make a difference
	} else {
		if (__builtin_expect(parent->NB::get_left() == node, true)) {
			parent->NB::set_left(nullptr);
			deleted_from_right = false;
		} else {
			parent->NB::set_right(nullptr);
			deleted_from_right = true;
		}
		NodeTraits::deleted_below(*parent, *this);
	}
	if constexpr (call_fixup) {
		this->fixup_after_delete(parent, deleted_from_right);
	}
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <bool call_fixup>
bool
WBTree<Node, NodeTraits, Options, Tag, Compare>::remove_swap_and_remove_right(
    Node * node, Node * replacement)
{
	// std::cout << " ----- SRR " << std::hex << node << " <> " << replacement <<
	// std::dec << "\n";
	bool deleted_from_right;
	Node * parent;

	this->swap_nodes(node, replacement);

	// Now, node is where replacement should point to
	replacement = node;
	parent = replacement->NB::get_parent();

	if (replacement->NB::get_right() != nullptr) {
		NodeTraits::splice_out_left_knee(*replacement, *this);
		// Update child's parent if there is such a child
		replacement->NB::get_right()->NB::set_parent(parent);
	} else {
		NodeTraits::delete_leaf(*replacement, *this);
	}

	if (__builtin_expect(parent == nullptr, false)) {
		this->root = replacement->NB::get_right();
		deleted_from_right = true; // doesn't make a difference
	} else {
		// TODO this is only dependent on the number of steps we took in the
		// loop above!
		if (__builtin_expect(parent->NB::get_left() == replacement, true)) {
			parent->NB::set_left(replacement->NB::get_right());
			deleted_from_right = false;
		} else {
			parent->NB::set_right(replacement->NB::get_right());
			deleted_from_right = true;
		}
		NodeTraits::deleted_below(*parent, *this);
	}

	if constexpr (call_fixup) {
		this->fixup_after_delete(parent, deleted_from_right);
	}

	return deleted_from_right;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <bool call_fixup>
bool
WBTree<Node, NodeTraits, Options, Tag, Compare>::remove_swap_and_remove_left(
    Node * node, Node * replacement)
{
	bool deleted_from_right;
	Node * parent;

	this->swap_nodes(node, replacement);

	// Now, node is where replacement should point to
	replacement = node;
	parent = replacement->NB::get_parent();

	if (replacement->NB::get_left() != nullptr) {
		NodeTraits::splice_out_right_knee(*replacement, *this);

		// Update child's parent if there is such a child
		replacement->NB::get_left()->NB::set_parent(parent);
	} else {
		NodeTraits::delete_leaf(*replacement, *this);
	}

	if (__builtin_expect(parent == nullptr, false)) {
		this->root = replacement->NB::get_left();
		deleted_from_right = true; // doesn't make a difference
	} else {
		// TODO this is only dependent on the number of steps we took in the
		// loop earlier!
		if (__builtin_expect(parent->NB::get_right() == replacement, true)) {
			parent->NB::set_right(replacement->NB::get_left());
			deleted_from_right = true;
		} else {
			parent->NB::set_left(replacement->NB::get_left());
			deleted_from_right = false;
		}
		NodeTraits::deleted_below(*parent, *this);
	}

	if constexpr (call_fixup) {
		this->fixup_after_delete(parent, deleted_from_right);
	}

	return deleted_from_right;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
WBTree<Node, NodeTraits, Options, Tag, Compare>::remove_to_leaf(Node & node)
{
	Node * cur = &node;

	// Size reduction is done during up-traversal!

	// TODO follow both paths and see where less fixups are necessary?
	// TODO make configurable whether right-leaning or left-leaning?
	// or use size? (currently implemented)
	if ((cur->NB::get_left() != nullptr) &&
	    ((cur->NB::get_right() == nullptr) ||
	     (cur->NB::get_left()->NB::_wbt_size >
	      cur->NB::get_right()->NB::_wbt_size))) {
		// Use the largest node on the left
		cur = cur->NB::get_left();
		while (cur->NB::get_right() != nullptr) {
			cur = cur->NB::get_right();
		}

		this->remove_swap_and_remove_left<true>(&node, cur);
		/*
		this->swap_nodes(&node, cur);

		// Now, node is where cur should point to
		cur = &node;
		parent = cur->NB::get_parent();

		if (cur->NB::get_left() != nullptr) {
		  NodeTraits::splice_out_right_knee(*cur, *this);

		  // Update child's parent if there is such a child
		  cur->NB::get_left()->NB::set_parent(parent);
		} else {
		  NodeTraits::delete_leaf(*cur, *this);
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
		*/
	} else if (cur->NB::get_right() != nullptr) {
		// use the smallest on the right
		cur = cur->NB::get_right();
		while (cur->NB::get_left() != nullptr) {
			cur = cur->NB::get_left();
		}

		this->remove_swap_and_remove_right<true>(&node, cur);
		/*
		this->swap_nodes(&node, cur);

		// Now, node is where cur should point to
		cur = &node;
		parent = cur->NB::get_parent();

		if (cur->NB::get_right() != nullptr) {
		  NodeTraits::splice_out_left_knee(*cur, *this);
		  // Update child's parent if there is such a child
		  cur->NB::get_right()->NB::set_parent(parent);
		} else {
		  NodeTraits::delete_leaf(*cur, *this);
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
		*/
	} else {
		// This is a leaf!
		this->remove_leaf<true>(cur);
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

			if ((typename Options::WBTDeltaT)((last_size + 1) *
			                                  Options::wbt_delta()) <
			    (typename Options::WBTDeltaT)(right_weight + 1)) {
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

				if ((typename Options::WBTGammaT)right_left >
				    (typename Options::WBTGammaT)(Options::wbt_gamma() * right_right)) {
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

			if ((typename Options::WBTDeltaT)((last_size + 1) *
			                                  Options::wbt_delta()) <
			    (typename Options::WBTDeltaT)(left_weight + 1)) {
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

				if ((typename Options::WBTGammaT)left_right >
				    (typename Options::WBTGammaT)(Options::wbt_gamma() * left_left)) {
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

	if constexpr (Options::wbt_single_pass) {
		this->remove_onepass<true>(node);
	} else {
		this->remove_to_leaf(node);
	}
}

} // namespace weight
} // namespace ygg

#endif // YGG_RBTREE_CPP
