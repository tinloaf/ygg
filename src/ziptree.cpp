#ifndef YGG_ZIPTREE_CPP
#define YGG_ZIPTREE_CPP

#include "ziptree.hpp"

#include <fstream>
#include <iostream>
#include <vector>

namespace ygg {

namespace ztree_internal {
// @cond INTERNAL

template <class Tree>
struct dbg_verify_size_helper<Tree, true>
{
	void
	operator()(const Tree & t, size_t node_count)
	{
		assert(t.size() == node_count);
		// Usage in assertion is not enough for GCC to realize that the variable is
		// being used.
		(void)t;
		(void)node_count;
	}
};

template <class Tree>
struct dbg_verify_size_helper<Tree, false>
{
	void
	operator()(const Tree & t, size_t node_count)
	{
		(void)t;
		(void)node_count;
	}
};

template <class Node, class Options>
ZTreeRankGenerator<Node, Options, true, false>::ZTreeRankGenerator()
{}

template <class Node, class Options>
void
ZTreeRankGenerator<Node, Options, true, false>::update_rank(
    Node & node) noexcept
{
	(void)node;
}

template <class Node, class Options>
int
ZTreeRankGenerator<Node, Options, true, false>::get_rank(
    const Node & node) noexcept
{
	// TODO ffsl? ffs?
	if constexpr (Options::ztree_universalize_lincong) {
		// TODO this is not strictly a universal family
		size_t universalized =
		    (std::hash<Node>{}(node)*Options::ztree_universalize_coefficient) %
		    Options::ztree_universalize_modul;
		return __builtin_ffsl(static_cast<long int>(universalized));
	} else if constexpr (Options::ztree_universalize_multiply) {
		// This is a variant of the multiply-shift method by Dietzfelbinger et al.
		// Since we hash to all of size_t, we don't need a shift.
		size_t universalized =
		    (std::hash<Node>{}(node)*Options::ztree_universalize_coefficient);
		return __builtin_ffsl(static_cast<long int>(universalized));
	} else {
		return __builtin_ffsl(static_cast<long int>(std::hash<Node>{}(node)));
	}
}

template <class Node, class Options>
ZTreeRankGenerator<Node, Options, true, true>::ZTreeRankGenerator()
{}

template <class Node, class Options>
void
ZTreeRankGenerator<Node, Options, true, true>::update_rank(Node & node) noexcept
{
	// TODO ffsl? ffs?
	if constexpr (Options::ztree_universalize_lincong) {
		// TODO this is not strictly a universal family
		size_t universalized =
		    (std::hash<Node>{}(node)*Options::ztree_universalize_coefficient) %
		    Options::ztree_universalize_modul;
		node._zt_rank.rank = static_cast<decltype(node._zt_rank.rank)>(
		    __builtin_ffsl(static_cast<long int>(universalized)));
	} else if constexpr (Options::ztree_universalize_multiply) {
		// This is a variant of the multiply-shift method by Dietzfelbinger et al.
		// Since we hash to all of size_t, we don't need a shift.
		size_t universalized =
		    (std::hash<Node>{}(node)*Options::ztree_universalize_coefficient);
		node._zt_rank.rank = static_cast<decltype(node._zt_rank.rank)>(
		    __builtin_ffsl(static_cast<long int>(universalized)));
	} else {
		node._zt_rank.rank = static_cast<decltype(node._zt_rank.rank)>(
		    __builtin_ffsl(static_cast<long int>(std::hash<Node>{}(node))));
	}
}

template <class Node, class Options>
size_t
ZTreeRankGenerator<Node, Options, true, true>::get_rank(
    const Node & node) noexcept
{
	return static_cast<size_t>(node._zt_rank.rank);
}

template <class Node, class Options>
ZTreeRankGenerator<Node, Options, false, true>::ZTreeRankGenerator()
{
	auto rand_val = std::rand();
	this->rank = 0;
	while (rand_val == RAND_MAX) {
		this->rank = static_cast<decltype(this->rank)>(
		    (this->rank + static_cast<decltype(this->rank)>(std::log2(RAND_MAX))));
		rand_val = std::rand();
	}
	this->rank = static_cast<decltype(this->rank)>(
	    __builtin_ffsl(static_cast<long int>(rand_val)));
}

template <class Node, class Options>
template <class URBG>
ZTreeRankGenerator<Node, Options, false, true>::ZTreeRankGenerator(URBG && g)
{
	auto rand_val = g();
	this->rank = 0;
	while (rand_val == g.max()) {
		this->rank += static_cast<size_t>(std::log2(g.max()));
		rand_val = g();
	}
	this->rank = __builtin_ffsl(static_cast<long int>(rand_val));
}

template <class Node, class Options>
void
ZTreeRankGenerator<Node, Options, false, true>::update_rank(
    Node & node) noexcept
{
	(void)node;
}

template <class Node, class Options>
size_t
ZTreeRankGenerator<Node, Options, false, true>::get_rank(
    const Node & node) noexcept
{
	return static_cast<size_t>(node._zt_rank.rank);
}

// @endcond
} // namespace ztree_internal

template <class Node, class Options, class Tag>
size_t
ZTreeNodeBase<Node, Options, Tag>::get_depth() const noexcept
{
	size_t depth = 0;
	const Node * n = static_cast<const Node *>(this);

	while (n->get_parent() != nullptr) {
		depth++;
		n = n->get_parent();
	}

	return depth;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::ZTree() noexcept
{}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::ZTree(
    MyClass && other) noexcept
{
	this->root = other.root;
	other.root = nullptr;
	this->s = other.s;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter> &
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::operator=(
    MyClass && other) noexcept
{
	this->root = other.root;
	other.root = nullptr;
	this->s = other.s;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::insert(
    Node & node) noexcept
{
#ifdef YGG_STORE_SEQUENCE
	this->bss.register_insert(reinterpret_cast<const void *>(&node),
	                          Options::SequenceInterface::get_key(node));
#endif

	// TODO set these only where necessary
	node.NB::set_parent(nullptr);
	node.NB::set_left(nullptr);
	node.NB::set_right(nullptr);

	// First, search for insertion position.
	auto node_rank = RankGetter::get_rank(node);
	this->s.add(1);

	// TODO this should be handled by the code below
	if (this->root == nullptr) {
		this->root = &node;
		return;
	}

	if (RankGetter::get_rank(node) >= RankGetter::get_rank(*this->root)) {
		// Replacing the root!
		Node * old_root = this->root;
		this->root = &node;

		this->unzip(*old_root, node);
	} else {
		Node * current = this->root;

		// Find the *parent* of the node to be replaced
		// TODO this assumes that comparison is cheaper than rank-getting. Should we
		// implement both?
		// TODO this assumes to insert the nodes as far to the bottom as possible in
		// the case of rank ties. Is that a good idea?
		while (true) {
			// Check if we must descend left

			bool goes_after = this->cmp(*current, node);
			if (!goes_after &&
			    __builtin_expect((current->NB::get_left() != nullptr), 1) &&
			    (RankGetter::get_rank(*current->NB::get_left()) >= node_rank)) {
				current = current->NB::get_left();
			} else if (goes_after &&
			           __builtin_expect((current->NB::get_right() != nullptr), 1) &&
			           (RankGetter::get_rank(*current->NB::get_right()) >=
			            node_rank)) {
				current = current->NB::get_right();
			} else {
				// we're done!
				break;
			}
		}

		// Place node below parent
		Node * old_node = nullptr;

		node.NB::set_parent(current);
		if (!this->cmp(*current, node)) {
			// Place left
			if (current->NB::get_left() != nullptr) {
				old_node = current->NB::get_left();
			}
			current->NB::set_left(&node);
		} else {
			// Place right
			if (current->NB::get_right() != nullptr) {
				old_node = current->NB::get_right();
			}
			current->NB::set_right(&node);
		}

		if (old_node != nullptr) {
			this->unzip(*old_node, node);
		}
	}
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare,
      RankGetter>::dbg_print_rank_stats() const
{
	std::vector<size_t> rank_count;

	for (auto & node : *this) {
		size_t rank = RankGetter::get_rank(node);
		if (rank_count.size() <= rank) {
			rank_count.resize(rank + 1, 0);
		}
		rank_count[rank]++;
	}

	for (unsigned int rank = 1; rank < rank_count.size(); ++rank) {
		std::cout << "Rank " << rank << "\t: " << rank_count[rank] << std::endl;
	}
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::unzip(
    Node & oldn, Node & newn) noexcept
{
	Node * left_head = &newn;
	Node * right_head = &newn;

	Node * cur = &oldn;

	NodeTraits traits;
	traits.init_unzipping(&newn);

	// std::cout << "========================\n";
	// std::cout << "Start unzipping with root " << (size_t)&newn << "\n";

	/*
	 * The following code has been micro-optimized in the big loop below:
	 *
	while (cur != nullptr) {
	  if (this->cmp(newn, *cur)) {
	    // Add to the right spine

	    if (__builtin_expect((right_head != &newn), 1)) {
	      right_head->_zt_left = cur;
	    } else {
	      right_head->_zt_right = cur;
	    }

	    cur->_zt_parent = right_head;
	    right_head = cur;

	    cur = cur->_zt_left;
	  } else {
	    // Add to the left spine

	    if (__builtin_expect((left_head != &newn), 1)) {
	      left_head->_zt_right = cur;
	    } else {
	      left_head->_zt_left = cur;
	    }

	    cur->_zt_parent = left_head;
	    left_head = cur;

	    cur = cur->_zt_right;
	  }
	}
	*/

	//
	// State: Neither left nor right spine have been started
	//
	if (this->cmp(newn, *cur)) {
		// Add to the right spine

		// Start the right spine
		traits.unzip_to_right(cur);
		right_head->NB::set_right(cur);

		cur->NB::set_parent(right_head);
		right_head = cur;

		cur = cur->NB::get_left();

		while (cur != nullptr) {
			//
			// State : Right spine has been started, left has not been started
			//

			if (this->cmp(newn, *cur)) {
				// Add to the right spine

				// Right spine has been started, add to the left
				traits.unzip_to_right(cur);
				right_head->NB::set_left(cur);

				cur->NB::set_parent(right_head);
				right_head = cur;

				cur = cur->NB::get_left();
			} else {

				// Start the left spine
				traits.unzip_to_left(cur);
				left_head->NB::set_left(cur);

				cur->NB::set_parent(left_head);
				left_head = cur;

				cur = cur->NB::get_right();

				while (cur != nullptr) {
					//
					// State: both spines have been started
					//
					if (this->cmp(newn, *cur)) {
						// Add to the right spine

						traits.unzip_to_right(cur);
						right_head->NB::set_left(cur);

						cur->NB::set_parent(right_head);
						right_head = cur;

						cur = cur->NB::get_left();
					} else {
						// Add to the left spine

						traits.unzip_to_left(cur);
						left_head->NB::set_right(cur);

						cur->NB::set_parent(left_head);
						left_head = cur;

						cur = cur->NB::get_right();
					}
				}
				break;
			}
		}

	} else {
		// Add to the left spine

		// Start the left spine
		traits.unzip_to_left(cur);
		left_head->NB::set_left(cur);

		cur->NB::set_parent(left_head);
		left_head = cur;

		cur = cur->NB::get_right();

		while (cur != nullptr) {
			//
			// State : Left spine has been started, right not
			//
			if (this->cmp(newn, *cur)) {
				// Add to the right spine

				// Start the right spine
				traits.unzip_to_right(cur);
				right_head->NB::set_right(cur);

				cur->NB::set_parent(right_head);
				right_head = cur;

				cur = cur->NB::get_left();

				while (cur != nullptr) {
					//
					// State: both spines have been started
					//
					if (this->cmp(newn, *cur)) {
						// Add to the right spine

						traits.unzip_to_right(cur);
						right_head->NB::set_left(cur);

						cur->NB::set_parent(right_head);
						right_head = cur;

						cur = cur->NB::get_left();
					} else {
						// Add to the left spine
						traits.unzip_to_left(cur);
						left_head->NB::set_right(cur);

						cur->NB::set_parent(left_head);
						left_head = cur;

						cur = cur->NB::get_right();
					}
				}
				break;

			} else {
				// Add to the left spine
				traits.unzip_to_left(cur);
				left_head->NB::set_right(cur);

				cur->NB::set_parent(left_head);
				left_head = cur;

				cur = cur->NB::get_right();
			}
		}
	}

	// End of the spines
	if (left_head != &newn) {
		left_head->NB::set_right(nullptr);
	} else {
		left_head->NB::set_left(nullptr);
	}

	if (right_head != &newn) {
		right_head->NB::set_left(nullptr);
	} else {
		right_head->NB::set_right(nullptr);
	}

	traits.unzip_done(&newn, left_head, right_head);
} // namespace ygg

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::remove(Node & n)
    CMP_NOEXCEPT(n)
{
#ifdef YGG_STORE_SEQUENCE
	this->bss.register_delete(reinterpret_cast<const void *>(&n),
	                          Options::SequenceInterface::get_key(n));
#endif

	this->s.reduce(1);
	this->zip(n);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
template <class Comparable>
Node *
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::erase(
    const Comparable & c) CMP_NOEXCEPT(c)
{
#ifdef YGG_STORE_SEQUENCE
	this->bss.register_erase(reinterpret_cast<const void *>(&c),
	                         Options::SequenceInterface::get_key(c));
#endif

	auto el = this->find(c);
	if (el != this->end()) {
		this->s.reduce(1);
		this->zip(*el);
		return &(*el);
	}
	return static_cast<Node *>(nullptr);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::zip(
    Node & old_root) noexcept
{
	NodeTraits traits;

	Node * left_head = old_root.NB::get_left();
	Node * right_head = old_root.NB::get_right();
	Node * new_head = nullptr;

	Node * cur = old_root.NB::get_parent();

	bool last_from_left;

	// First one is special.

	// Left child is used if
	//  - right is empty *or*
	//  - left is nonempty *and* has higher rank
	// TODO this leads to a left-leaning behavior. Correct this?
	if ((right_head == nullptr) ||
	    ((left_head != nullptr) && (RankGetter::get_rank(*left_head) >
	                                RankGetter::get_rank(*right_head)))) {
		if (left_head == nullptr) {
			// Both child-trees are empty. This is a special case: Just remove the
			// node and return, no zipping necessary.
			traits.delete_without_zipping(&old_root);

			if (cur == nullptr) {
				this->root = nullptr;
			} else {
				if (cur->NB::get_left() == &old_root) {
					cur->NB::set_left(nullptr);
				} else {
					assert(cur->NB::get_right() == &old_root);
					cur->NB::set_right(nullptr);
				}
			}
			return;
		}

		traits.init_zipping(&old_root);

		// use left
		traits.before_zip_from_left(left_head);
		last_from_left = true;
		new_head = left_head;

		if (cur == nullptr) {
			this->root = left_head;
			left_head->NB::set_parent(nullptr);
		} else {
			if (cur->NB::get_left() == &old_root) {
				cur->NB::set_left(left_head);
			} else {
				assert(cur->NB::get_right() == &old_root);
				cur->NB::set_right(left_head);
			}
			left_head->NB::set_parent(cur);
		}
		cur = left_head;
		left_head = left_head->NB::get_right();
	} else {
		traits.init_zipping(&old_root);

		last_from_left = false;

		// use right
		traits.before_zip_from_right(right_head);
		new_head = right_head;

		if (cur == nullptr) {
			this->root = right_head;
			right_head->NB::set_parent(nullptr);
		} else {
			if (cur->NB::get_left() == &old_root) {
				cur->NB::set_left(right_head);
			} else {
				assert(cur->NB::get_right() == &old_root);
				cur->NB::get_right() = right_head;
			}

			right_head->NB::set_parent(cur);
		}
		cur = right_head;
		right_head = right_head->NB::get_left();
	}

	// TODO this leads to a left-leaning behavior. Correct this?
	while ((left_head != nullptr) && (right_head != nullptr)) {
		if ((RankGetter::get_rank(*left_head) >
		     RankGetter::get_rank(*right_head))) {
			// Use left
			traits.before_zip_from_left(left_head);

			if (last_from_left) {
				// just pass on
			} else {
				// last was from the right, we can just use its left child.
				cur->NB::set_left(left_head);
				left_head->NB::set_parent(cur);
			}

			cur = left_head;
			left_head = left_head->NB::get_right();
			last_from_left = true;
		} else {
			// use right
			traits.before_zip_from_right(right_head);

			if (!last_from_left) {
				// just pass on
			} else {
				// last was from the left, we can just use its right child.
				cur->NB::set_right(right_head);
				right_head->NB::set_parent(cur);
			}
			cur = right_head;
			right_head = right_head->NB::get_left();
			last_from_left = false;
		}
	}

	// If one of both heads has become nullptr, the other tree might still be
	// non-empty. We must re-hang this one completely.
	if (left_head != nullptr) {
		// Right head is nullptr, re-hang left tree
		if (!last_from_left) {
			traits.before_zip_tree_from_left(left_head);
			cur->NB::set_left(left_head);
			left_head->NB::set_parent(cur);
			cur = left_head;
		} else {
			traits.zipping_ended_left_without_tree(cur);
		}
	} else if (right_head != nullptr) {
		if (last_from_left) {
			traits.before_zip_tree_from_right(right_head);
			cur->NB::set_right(right_head);
			right_head->NB::set_parent(cur);
			cur = right_head;
		} else {
			traits.zipping_ended_right_without_tree(cur);
		}
	} else {
		if (last_from_left) {
			traits.zipping_ended_left_without_tree(cur);
		} else {
			traits.zipping_ended_right_without_tree(cur);
		}
	}

	if (cur == nullptr) {
		cur = new_head;
	}
	traits.zipping_done(new_head, cur);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::dbg_verify() const
{
	if (this->root != nullptr) {
		assert(this->root->get_parent() == nullptr);
	}

	this->dbg_verify_consistency(this->root, nullptr, nullptr);
	if (Options::constant_time_size) {
		this->dbg_verify_size();
	}
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::dbg_verify_size()
    const
{
	size_t node_count = 0;
	for (auto & node : *this) {
		(void)node;
		node_count++;
	}

	ztree_internal::dbg_verify_size_helper<MyClass,
	                                       Options::constant_time_size>{}(
	    *this, node_count);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare,
      RankGetter>::dbg_verify_consistency(Node * sub_root,
                                          Node * lower_bound_node,
                                          Node * upper_bound_node) const
{
	if (sub_root == nullptr) {
		return;
	}

	if (sub_root->NB::get_parent() == nullptr) {
		assert(this->root == sub_root);
	} else {
		assert(this->root != sub_root);
	}

	assert(sub_root->NB::get_parent() != sub_root);

	if (lower_bound_node != nullptr) {
		assert(this->cmp(*lower_bound_node, *sub_root));
	}
	if (upper_bound_node != nullptr) {
		assert(!this->cmp(*upper_bound_node, *sub_root));
	}

	if (sub_root->NB::get_right() != nullptr) {
		assert(RankGetter::get_rank(*sub_root->NB::get_right()) <=
		       RankGetter::get_rank(*sub_root));
		assert(this->cmp(*sub_root, *sub_root->NB::get_right()));
		assert(sub_root->NB::get_right()->NB::get_parent() == sub_root);

		this->dbg_verify_consistency(sub_root->NB::get_right(), sub_root,
		                             upper_bound_node);
	}

	if (sub_root->NB::get_left() != nullptr) {
		assert(RankGetter::get_rank(*sub_root->NB::get_left()) <=
		       RankGetter::get_rank(*sub_root));
		assert(!this->cmp(*sub_root, *sub_root->NB::get_left()));
		assert(sub_root->NB::get_left()->NB::get_parent() == sub_root);

		this->dbg_verify_consistency(sub_root->NB::get_left(), lower_bound_node,
		                             sub_root);
	}
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::dump_to_dot(
    const std::string & filename) const
{
	this->dump_to_dot_base(
	    filename, [&](const Node * node) { return NodeTraits::get_id(node); });
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
template <class NodeNameGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::dump_to_dot_base(
    const std::string & filename, NodeNameGetter name_getter) const
{
	std::ofstream dotfile;
	dotfile.open(filename);

	dotfile << "digraph G {\n";
	if (this->root != nullptr) {
		this->output_node_base(this->root, dotfile, name_getter);
	}
	dotfile << "}\n";
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
template <class NodeNameGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::output_node_base(
    const Node * node, std::ofstream & out, NodeNameGetter name_getter) const
{
	if (node == nullptr) {
		return;
	}

	std::string node_label = name_getter(node);
	node_label += " (";
	node_label += std::to_string(RankGetter::get_rank(*node));
	node_label += ")";

	out << "  " << std::hex << node << std::dec << "[ label=\"" << node_label
	    << "\"]\n";

	if (node->NB::get_parent() != nullptr) {
		std::string label;
		if (node->NB::get_parent()->NB::get_left() == node) {
			label = std::string("L");
		} else {
			label = std::string("R");
		}

		out << "  " << std::hex << node->NB::get_parent() << std::dec << " -> "
		    << std::hex << node << std::dec << "[ label=\"" << label << "\"]\n";
	}

	this->output_node_base(node->NB::get_left(), out, name_getter);
	this->output_node_base(node->NB::get_right(), out, name_getter);
}

} // namespace ygg

#endif // YGG_ZIPTREE_CPP
