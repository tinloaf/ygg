#ifndef YGG_AVGMINTREE_CPP
#define YGG_AVGMINTREE_CPP

#include <cstddef>
#include <fstream>
#include <iostream>
#include <vector>

#include "avgmintree.hpp"
#include "debug.hpp"

namespace ygg {

template <class Node, class Options, class Tag>
size_t
AvgMinTreeNodeBase<Node, Options, Tag>::get_depth() const noexcept
{
	size_t depth = 0;
	const Node * n = (const Node *)this;

	while (n->get_parent() != nullptr) {
		depth++;
		n = n->get_parent();
	}

	return depth;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::AvgMinTree() noexcept
    : root(nullptr), cmp()
{}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::insert(
    Node & node) noexcept
{
	this->s.add(1);

	// TODO set these only where necessary
	node._zt_parent = nullptr;
	node._zt_left = nullptr;
	node._zt_right = nullptr;
	node.size = 1;
	node.length_sum = 0;

	/*
	 * Search for insertion position. What we do is:
	 *
	 * * Traverse the full search path down into the leaves
	 * * Traverse it back up, computing all average path lengths
	 * * Find the position that minimizes the maximum average path length
	 *   among all the nodes on the search path
	 *
	 */

	// The bool indicates whether the node goes left of <node>
	std::vector<std::pair<Node *, bool>> path;

	Node * current = this->root;
	while (current != nullptr) {
		// Check if we must descend left
		bool goes_after = this->cmp(*current, node);
		path.emplace_back(current, goes_after);

		if (!goes_after) {
			current = current->_zt_left;
		} else {
			current = current->_zt_right;
		}
	}

	std::vector<size_t> right_path_sum(path.size() + 1, 0);
	std::vector<size_t> right_size(path.size() + 1, 0);
	std::vector<size_t> left_path_sum(path.size() + 1, 0);
	std::vector<size_t> left_size(path.size() + 1, 0);
	std::vector<size_t> total_path_sum(path.size() + 1, 0);
	std::vector<int> path_sum_change(path.size() + 1, 0);

	if (path.empty()) {
		// Insert a new root
		this->root = &node;
		return;
	}

	total_path_sum.back() = path.size();
	path_sum_change.back() = path.size();

	int best_change = path.size();
	size_t best_position = path.size();

	for (int i = path.size() - 1; i >= 0; --i) {
		Node * n = path[i].first;
		bool left = path[i].second;

		right_path_sum[i] = right_path_sum[i + 1];
		right_size[i] = right_size[i + 1];

		left_path_sum[i] = left_path_sum[i + 1];
		left_size[i] = left_size[i + 1];

		if (left) {
			// This vertex goes to the left
			left_size[i] += 1;
			// All previous path lengths on the left are increased by 1
			left_path_sum[i] += left_size[i];

			if (n->_zt_left != nullptr) {
				// The left subtree of <n> also contributes to the weight of the
				// left spine
				left_size[i] += n->_zt_left->size;
				// All the paths from <n->_zt_left> to its successors are part of the
				// left spine, plus one edge each
				left_path_sum[i] += (n->_zt_left->length_sum) + (n->_zt_left->size);
			}
		} else {
			right_size[i] += 1;
			right_path_sum[i] += right_size[i];

			if (n->_zt_right != nullptr) {
				right_size[i] += n->_zt_right->size;
				right_path_sum[i] += (n->_zt_right->length_sum) + (n->_zt_right->size);
			}
		}

		total_path_sum[i] = left_path_sum[i] + (left_size[i] * i) +
		                    right_path_sum[i] + (right_size[i] * i) + i;

		size_t previous_path_sum = (n->size * i) + n->length_sum;
		path_sum_change[i] = (int)total_path_sum[i] - (int)previous_path_sum;
		if (path_sum_change[i] < best_change) {
			best_change = path_sum_change[i];
			best_position = i;
		}
	}

	// Actually insert
	if (best_position == 0) {
		// replacing the root!
		this->root = &node;
		this->unzip(*(path[0].first), node);
	} else if (best_position == path.size()) {
		// Inserting a leaf
		Node * parent = path[best_position - 1].first;
		bool parent_left_of_node = path[best_position - 1].second;

		node._zt_parent = parent;
		if (parent_left_of_node) {
			parent->_zt_right = &node;
		} else {
			parent->_zt_left = &node;
		}

		/* From the leave on up, update sizes and path sums! */
		size_t i = 1;
		while (parent != nullptr) {
			parent->size += 1;
			parent->length_sum += i++;
			parent = parent->get_parent();
		}
	} else {
		// Hang node under its new parent
		Node * parent = path[best_position - 1].first;
		bool parent_left_of_node = path[best_position - 1].second;
		Node * unzip_at = path[best_position].first;

		node._zt_parent = parent;
		if (parent_left_of_node) {
			parent->_zt_right = &node;
		} else {
			parent->_zt_left = &node;
		}

		this->unzip(*unzip_at, node);

		/* From the inserted node on up, update sizes and path sums!
		 * Up to and including the inserted node, unzip() should have taken
		 * care of this. */
		/* TODO Reconstruction *should* be unnecessary */
		while (parent != nullptr) {
			parent->size = 1;
			parent->length_sum = 0;

			if (parent->get_left() != nullptr) {
				parent->size += parent->get_left()->size;
				parent->length_sum +=
				    parent->get_left()->length_sum + parent->get_left()->size;
			}

			if (parent->get_right() != nullptr) {
				parent->size += parent->get_right()->size;
				parent->length_sum +=
				    parent->get_right()->length_sum + parent->get_right()->size;
			}

			parent = parent->get_parent();
		}
	}
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
Node *
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::get_root() const
{
	return this->root;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::unzip(Node & oldn,
                                                           Node & newn) noexcept
{
	Node * left_head = &newn;
	Node * right_head = &newn;

	Node * cur = &oldn;

	NodeTraits traits;
	traits.init_unzipping(&newn);

	// std::cout << "========================\n";
	// std::cout << "Start unzipping with root " << (size_t)&newn << "\n";

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

	if (left_head != &newn) {
		left_head->_zt_right = nullptr;
	}
	if (right_head != &newn) {
		right_head->_zt_left = nullptr;
	}

	/* Reconstruct */
	while (left_head != &newn) {
		left_head->size = 1;
		left_head->length_sum = 0;

		if (left_head->get_left() != nullptr) {
			left_head->size += left_head->get_left()->size;
			left_head->length_sum +=
			    left_head->get_left()->length_sum + (left_head->get_left()->size);
		}

		if (left_head->get_right() != nullptr) {
			left_head->size += left_head->get_right()->size;
			left_head->length_sum +=
			    left_head->get_right()->length_sum + (left_head->get_right()->size);
		}

		left_head = left_head->get_parent();
	}

	while (right_head != &newn) {
		right_head->size = 1;
		right_head->length_sum = 0;

		if (right_head->get_left() != nullptr) {
			right_head->size += right_head->get_left()->size;
			right_head->length_sum +=
			    right_head->get_left()->length_sum + (right_head->get_left()->size);
		}

		if (right_head->get_right() != nullptr) {
			right_head->size += right_head->get_right()->size;
			right_head->length_sum +=
			    right_head->get_right()->length_sum + (right_head->get_right()->size);
		}

		right_head = right_head->get_parent();
	}

	newn.size = 1;
	newn.length_sum = 0;
	if (newn.get_left() != nullptr) {
		newn.size += newn.get_left()->size;
		newn.length_sum += newn.get_left()->length_sum + (newn.get_left()->size);
	}

	if (newn.get_right() != nullptr) {
		newn.size += newn.get_right()->size;
		newn.length_sum += newn.get_right()->length_sum + (newn.get_right()->size);
	}

} // namespace ygg

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::remove(Node & n) noexcept
{
	this->s.reduce(1);
	Node * old_parent = n.get_parent();
	this->zip(n);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::zip(
    Node & old_root) noexcept
{
	NodeTraits traits;

	Node * left_head = old_root._zt_left;
	Node * right_head = old_root._zt_right;
	Node * new_head = nullptr;

	Node * cur = old_root._zt_parent;

	bool last_from_left;
	bool first = true; // TODO unroll

	if ((left_head == nullptr) && (right_head == nullptr)) {
		// Special case - no zipping
		traits.delete_without_zipping(&old_root);

		if (cur == nullptr) {
			this->root = nullptr;
		} else {
			if (cur->_zt_left == &old_root) {
				cur->_zt_left = nullptr;
			} else {
				assert(cur->_zt_right == &old_root);
				cur->_zt_right = nullptr;
			}
		}
	}

	while ((left_head != nullptr) && (right_head != nullptr)) {
		if (left_head->size < right_head->size) {
			// Next vertex comes from the left
			if (first) {
				first = false;
				new_head = left_head;
				if (cur == nullptr) {
					// Replacing root
					this->root = left_head;
				} else {
					if (cur->_zt_right == &old_root) {
						cur->_zt_right = left_head;
					} else {
						assert(cur->_zt_left == &old_root); // TODO remove
						cur->_zt_left = left_head;
					}
				}
			} else {
				if (last_from_left) {
					cur->_zt_right = left_head;
				} else {
					cur->_zt_left = left_head;
				}
			}
			left_head->_zt_parent = cur;
			cur = left_head;
			left_head = left_head->_zt_right;
			last_from_left = true;
		} else {
			if (first) {
				first = false;
				new_head = right_head;
				if (cur == nullptr) {
					// Replacing root
					this->root = right_head;
				} else {
					if (cur->_zt_right == &old_root) {
						cur->_zt_right = right_head;
					} else {
						assert(cur->_zt_left == &old_root); // TODO remove
						cur->_zt_left = right_head;
					}
				}
			} else {
				if (!last_from_left) {
					cur->_zt_left = right_head;
				} else {
					cur->_zt_right = right_head;
				}
			}
			right_head->_zt_parent = cur;
			cur = right_head;
			right_head = right_head->_zt_left;
			last_from_left = false;
		}
	}

	// If one of both heads has become nullptr, the other tree might still be
	// non-empty. We must re-hang this one completely.
	if (left_head != nullptr) {
		// Right head is nullptr, re-hang left tree
		if (first) {
			traits.before_zip_tree_from_left(left_head);
			new_head = left_head;
			if (cur == nullptr) {
				this->root = left_head;
				left_head->_zt_parent = nullptr;
			} else {
				if (cur->_zt_left == &old_root) {
					cur->_zt_left = left_head;
				} else {
					assert(cur->_zt_right == &old_root);
					cur->_zt_right = left_head;
				}
				left_head->_zt_parent = cur;
			}
			cur = left_head;
		} else if (!last_from_left) {
			traits.before_zip_tree_from_left(left_head);

			if (cur != nullptr) {
				cur->_zt_left = left_head;
				left_head->_zt_parent = cur;
			} else {
				this->root = left_head;
				left_head->_zt_parent = nullptr;
			}
			cur = left_head;
		} else {
			traits.zipping_ended_left_without_tree(cur);
		}
	} else if (right_head != nullptr) {
		if (first) {
			traits.before_zip_tree_from_right(right_head);
			new_head = right_head;
			if (cur == nullptr) {
				this->root = right_head;
				right_head->_zt_parent = nullptr;
			} else {
				if (cur->_zt_left == &old_root) {
					cur->_zt_left = right_head;
				} else {
					assert(cur->_zt_right == &old_root);
					cur->_zt_right = right_head;
				}
				right_head->_zt_parent = cur;
			}
			cur = right_head;
		} else if (last_from_left) {
			traits.before_zip_tree_from_right(right_head);

			if (cur != nullptr) {
				cur->_zt_right = right_head;
				right_head->_zt_parent = cur;
			} else {
				this->root = right_head;
				right_head->_zt_parent = nullptr;
			}
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
	/*
	if (new_head != nullptr) {
	  std::cout << "Reconstruction!\n";
	  */
	/* Reconstruct */
	// Reconstruction must go up to the root
	Node * reconstruct = cur;
	while (reconstruct != nullptr) {
		reconstruct->size = 1;
		reconstruct->length_sum = 0;

		if (reconstruct->get_left() != nullptr) {
			reconstruct->size += reconstruct->get_left()->size;
			reconstruct->length_sum +=
			    reconstruct->get_left()->length_sum + (reconstruct->get_left()->size);
		}

		if (reconstruct->get_right() != nullptr) {
			reconstruct->size += reconstruct->get_right()->size;
			reconstruct->length_sum += reconstruct->get_right()->length_sum +
			                           (reconstruct->get_right()->size);
		}

		reconstruct = reconstruct->get_parent();
	}

	traits.zipping_done(new_head, cur);
} // namespace ygg

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::dbg_verify() const
{
	if (this->root != nullptr) {
		assert(this->root->get_parent() == nullptr);

		this->dbg_verify_path_lengths(this->root);
	}

	this->dbg_verify_consistency(this->root, nullptr, nullptr);
	if (Options::constant_time_size) {
		this->dbg_verify_size();
	}
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::dbg_verify_size() const
{
	size_t node_count = 0;
	for (auto & node : *this) {
		(void)node;
		node_count++;
	}

	avgmintree_internal::dbg_verify_size_helper<my_type,
	                                            Options::constant_time_size>{}(
	    *this, node_count);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::dbg_print() const
{
	avgmintree_internal::NodeNameGetter<Node, NodeTraits> nng;
	debug::TreePrinter<Node, decltype(nng)> tp(this->root, nng);
	tp.print();
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::dbg_verify_path_lengths(
    Node * sub_root) const
{
	size_t size = 1;
	size_t length_sum = 0;

	if (sub_root->get_left() != nullptr) {
		size += sub_root->get_left()->size;
		length_sum += sub_root->get_left()->length_sum + sub_root->get_left()->size;

		this->dbg_verify_path_lengths(sub_root->get_left());
	}

	if (sub_root->get_right() != nullptr) {
		size += sub_root->get_right()->size;
		length_sum +=
		    sub_root->get_right()->length_sum + sub_root->get_right()->size;

		this->dbg_verify_path_lengths(sub_root->get_right());
	}

	assert(sub_root->size == size);
	assert(sub_root->length_sum == length_sum);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::dbg_verify_consistency(
    Node * sub_root, Node * lower_bound, Node * upper_bound) const
{
	if (sub_root == nullptr) {
		return;
	}

	if (sub_root->_zt_parent == nullptr) {
		assert(this->root == sub_root);
	} else {
		assert(this->root != sub_root);
	}

	assert(sub_root->_zt_parent != sub_root);

	if (lower_bound != nullptr) {
		assert(this->cmp(*lower_bound, *sub_root));
	}
	if (upper_bound != nullptr) {
		assert(!this->cmp(*upper_bound, *sub_root));
	}

	if (sub_root->_zt_right != nullptr) {
		assert(this->cmp(*sub_root, *sub_root->_zt_right));
		assert(sub_root->_zt_right->_zt_parent == sub_root);

		this->dbg_verify_consistency(sub_root->_zt_right, sub_root, upper_bound);
	}

	if (sub_root->_zt_left != nullptr) {
		assert(!this->cmp(*sub_root, *sub_root->_zt_left));
		assert(sub_root->_zt_left->_zt_parent == sub_root);

		this->dbg_verify_consistency(sub_root->_zt_left, lower_bound, sub_root);
	}
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::dump_to_dot(
    const std::string & filename) const
{
	this->dump_to_dot_base(
	    filename, [&](const Node * node) { return NodeTraits::get_id(node); });
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <class NodeNameGetter>
void
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::dump_to_dot_base(
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

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <class NodeNameGetter>
void
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::output_node_base(
    const Node * node, std::ofstream & out, NodeNameGetter name_getter) const
{
	if (node == nullptr) {
		return;
	}

	std::string node_label = name_getter(node);

	out << "  " << std::to_string((long unsigned int)node) << "[ label=\""
	    << node_label << "\"]\n";

	if (node->NB::_zt_parent != nullptr) {
		std::string label;
		if (node->NB::_zt_parent->NB::_zt_left == node) {
			label = std::string("L");
		} else {
			label = std::string("R");
		}

		out << "  " << std::to_string((long unsigned int)node->NB::_zt_parent)
		    << " -> " << std::to_string((long unsigned int)node) << "[ label=\""
		    << label << "\"]\n";
	}

	this->output_node_base(node->NB::_zt_left, out, name_getter);
	this->output_node_base(node->NB::_zt_right, out, name_getter);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <class Comparable>
typename AvgMinTree<Node, NodeTraits, Options, Tag,
                    Compare>::template iterator<false>
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::lower_bound(
    const Comparable & query)
{
	Node * cur = this->root;
	Node * last_left = nullptr;

	while (cur != nullptr) {
		if (this->cmp(*cur, query)) {
			cur = cur->NB::_zt_right;
		} else {
			last_left = cur;
			cur = cur->NB::_zt_left;
		}
	}

	if (last_left != nullptr) {
		return iterator<false>(last_left);
	} else {
		return this->end();
	}
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <class Comparable>
typename AvgMinTree<Node, NodeTraits, Options, Tag,
                    Compare>::template iterator<false>
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::upper_bound(
    const Comparable & query)
{
	Node * cur = this->root;
	Node * last_left = nullptr;

	while (cur != nullptr) {
		if (this->cmp(query, *cur)) {
			last_left = cur;
			cur = cur->_zt_left;
		} else {
			cur = cur->_zt_right;
		}
	}

	if (last_left != nullptr) {
		return iterator<false>(last_left);
	} else {
		return this->end();
	}
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <class Comparable>
typename AvgMinTree<Node, NodeTraits, Options, Tag,
                    Compare>::template const_iterator<false>
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::upper_bound(
    const Comparable & query) const
{
	return const_iterator<false>(
	    const_cast<AvgMinTree<Node, NodeTraits, Options, Tag, Compare> *>(this)
	        ->upper_bound(query));
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <class Comparable>
typename AvgMinTree<Node, NodeTraits, Options, Tag,
                    Compare>::template const_iterator<false>
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::lower_bound(
    const Comparable & query) const
{
	return const_iterator<false>(
	    const_cast<AvgMinTree<Node, NodeTraits, Options, Tag, Compare> *>(this)
	        ->lower_bound(query));
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <class Comparable>
typename AvgMinTree<Node, NodeTraits, Options, Tag,
                    Compare>::template iterator<false>
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::find(
    const Comparable & query)
{
	Node * cur = this->root;
	Node * last_left = nullptr;

	while (cur != nullptr) {
		if (this->cmp(*cur, query)) {
			cur = cur->NB::_zt_right;
		} else {
			last_left = cur;
			cur = cur->NB::_zt_left;
		}
	}

	if ((last_left != nullptr) && (!this->cmp(query, *last_left))) {
		return iterator<false>(last_left);
	} else {
		return this->end();
	}
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <class Comparable>
typename AvgMinTree<Node, NodeTraits, Options, Tag,
                    Compare>::template const_iterator<false>
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::find(
    const Comparable & query) const
{
	return const_iterator<false>(const_cast<decltype(this)>(this)->find(query));
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
Node *
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::get_smallest() const
{
	Node * smallest = this->root;
	if (smallest == nullptr) {
		return nullptr;
	}

	while (smallest->NB::_zt_left != nullptr) {
		smallest = smallest->NB::_zt_left;
	}

	return smallest;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
Node *
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::get_largest() const
{
	Node * largest = this->root;
	if (largest == nullptr) {
		return nullptr;
	}

	while (largest->NB::_zt_right != nullptr) {
		largest = largest->NB::_zt_right;
	}

	return largest;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename AvgMinTree<Node, NodeTraits, Options, Tag,
                    Compare>::template const_iterator<false>
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::iterator_to(
    const Node & node) const
{
	return const_iterator<false>(&node);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename AvgMinTree<Node, NodeTraits, Options, Tag,
                    Compare>::template iterator<false>
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::iterator_to(Node & node)
{
	return iterator<false>(&node);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename AvgMinTree<Node, NodeTraits, Options, Tag,
                    Compare>::template const_iterator<false>
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::cbegin() const
{
	Node * smallest = this->get_smallest();
	if (smallest == nullptr) {
		return const_iterator<false>(nullptr);
	}

	return const_iterator<false>(smallest);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename AvgMinTree<Node, NodeTraits, Options, Tag,
                    Compare>::template const_iterator<false>
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::cend() const
{
	return const_iterator<false>(nullptr);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename AvgMinTree<Node, NodeTraits, Options, Tag,
                    Compare>::template const_iterator<false>
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::begin() const
{
	return this->cbegin();
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename AvgMinTree<Node, NodeTraits, Options, Tag,
                    Compare>::template iterator<false>
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::begin()
{
	Node * smallest = this->get_smallest();
	if (smallest == nullptr) {
		return iterator<false>(nullptr);
	}

	return iterator<false>(smallest);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename AvgMinTree<Node, NodeTraits, Options, Tag,
                    Compare>::template const_iterator<false>
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::end() const
{
	return this->cend();
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename AvgMinTree<Node, NodeTraits, Options, Tag,
                    Compare>::template iterator<false>
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::end()
{
	return iterator<false>(nullptr);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename AvgMinTree<Node, NodeTraits, Options, Tag,
                    Compare>::template const_iterator<true>
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::crbegin() const
{
	Node * largest = this->get_largest();
	if (largest == nullptr) {
		return const_iterator<true>(nullptr);
	}

	return const_iterator<true>(largest);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename AvgMinTree<Node, NodeTraits, Options, Tag,
                    Compare>::template const_iterator<true>
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::crend() const
{
	return const_iterator<true>(nullptr);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename AvgMinTree<Node, NodeTraits, Options, Tag,
                    Compare>::template const_iterator<true>
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::rbegin() const
{
	return this->crbegin();
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename AvgMinTree<Node, NodeTraits, Options, Tag,
                    Compare>::template const_iterator<true>
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::rend() const
{
	return this->crend();
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename AvgMinTree<Node, NodeTraits, Options, Tag,
                    Compare>::template iterator<true>
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::rbegin()
{
	Node * largest = this->get_largest();
	if (largest == nullptr) {
		return iterator<true>(nullptr);
	}

	return iterator<true>(largest);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename AvgMinTree<Node, NodeTraits, Options, Tag,
                    Compare>::template iterator<true>
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::rend()
{
	return iterator<true>(nullptr);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
size_t
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::size() const
{
	return this->s.get();
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
bool
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::empty() const
{
	return (this->root == nullptr);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
AvgMinTree<Node, NodeTraits, Options, Tag, Compare>::clear()
{
	this->root = nullptr;
	this->s.set(0);
}

} // namespace ygg

#endif
