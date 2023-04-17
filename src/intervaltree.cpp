// Only for flymake / cquery
#include "intervaltree.hpp"

namespace ygg {
namespace intervaltree_internal {

template <class Node, class NodeTraits, bool sort_upper>
template <class T1, class T2>
bool
IntervalCompare<Node, NodeTraits, sort_upper>::operator()(const T1 & lhs,
                                                          const T2 & rhs) const
{
	if constexpr (sort_upper) {
		if (NodeTraits::get_lower(lhs) < NodeTraits::get_lower(rhs)) {
			return true;
		} else if ((NodeTraits::get_lower(lhs) == NodeTraits::get_lower(rhs)) &&
		           (NodeTraits::get_upper(lhs) < NodeTraits::get_upper(rhs))) {
			return true;
		} else {
			return false;
		}
	} else {
		return NodeTraits::get_lower(lhs) < NodeTraits::get_lower(rhs);
	}
}

template <class Node, class INB, class NodeTraits>
template <class BaseTree>
void
ExtendedNodeTraits<Node, INB, NodeTraits>::leaf_inserted(Node & node,
                                                         BaseTree & t)
{
	(void)t;

	node.INB::_it_max_upper = NodeTraits::get_upper(node);

	// Propagate up
	Node * cur = node.get_parent();
	while ((cur != nullptr) &&
	       (cur->INB::_it_max_upper < node.INB::_it_max_upper)) {
		cur->INB::_it_max_upper = node.INB::_it_max_upper;
		cur = cur->get_parent();
	}
}

template <class Node, class INB, class NodeTraits>
void
ExtendedNodeTraits<Node, INB, NodeTraits>::fix_node(Node & node)
{
	auto old_val = node.INB::_it_max_upper;
	node.INB::_it_max_upper = NodeTraits::get_upper(node);

	if (node.get_left() != nullptr) {
		node.INB::_it_max_upper =
		    std::max(node.INB::_it_max_upper, node.get_left()->INB::_it_max_upper);
	}

	if (node.get_right() != nullptr) {
		node.INB::_it_max_upper =
		    std::max(node.INB::_it_max_upper, node.get_right()->INB::_it_max_upper);
	}

	if (old_val != node.INB::_it_max_upper) {
		// propagate up
		Node * cur = node.get_parent();
		if (cur != nullptr) {
			if ((cur->INB::_it_max_upper < node.INB::_it_max_upper) ||
			    (cur->INB::_it_max_upper == old_val)) {
				fix_node(*cur);
			}
		}
	}
}

template <class Node, class INB, class NodeTraits>
template <class BaseTree>
void
ExtendedNodeTraits<Node, INB, NodeTraits>::rotated_left(Node & node,
                                                        BaseTree & t)
{
	(void)t;

	// 'node' is the node that was the old parent.
	fix_node(node);
	fix_node(*(node.get_parent()));
}

template <class Node, class INB, class NodeTraits>
template <class BaseTree>
void
ExtendedNodeTraits<Node, INB, NodeTraits>::rotated_right(Node & node,
                                                         BaseTree & t)
{
	(void)t;

	// 'node' is the node that was the old parent.
	fix_node(node);
	fix_node(*(node.get_parent()));
}

template <class Node, class INB, class NodeTraits>
template <class BaseTree>
void
ExtendedNodeTraits<Node, INB, NodeTraits>::deleted_below(Node & node,
                                                         BaseTree & t)
{
	(void)t;

	fix_node(node);
}

template <class Node, class INB, class NodeTraits>
template <class BaseTree>
void
ExtendedNodeTraits<Node, INB, NodeTraits>::swapped(Node & n1, Node & n2,
                                                   BaseTree & t)
{
	(void)t;

	fix_node(n1);
	if (n1.get_parent() != nullptr) {
		fix_node(*(n1.get_parent()));
	}

	fix_node(n2);
	if (n2.get_parent() != nullptr) {
		fix_node(*(n2.get_parent()));
	}
}

template <class Node, class INB, class NodeTraits>
typename NodeTraits::key_type
ExtendedNodeTraits<Node, INB, NodeTraits>::get_lower(
    const intervaltree_internal::DummyRange<typename NodeTraits::key_type> &
        range)
{
	return std::get<0>(range);
}

template <class Node, class INB, class NodeTraits>
typename NodeTraits::key_type
ExtendedNodeTraits<Node, INB, NodeTraits>::get_upper(
    const intervaltree_internal::DummyRange<typename NodeTraits::key_type> &
        range)
{
	return std::get<1>(range);
}
} // namespace intervaltree_internal

template <class Node, class NodeTraits, class Options, class Tag>
IntervalTree<Node, NodeTraits, Options, Tag>::IntervalTree()
{}

template <class Node, class NodeTraits, class Options, class Tag>
bool
IntervalTree<Node, NodeTraits, Options, Tag>::verify_integrity() const
{
	bool base_verification = this->BaseTree::verify_integrity();
	assert(base_verification);
	bool maxima_valid =
	    this->root == nullptr ? true : this->verify_maxima(this->root);
	assert(maxima_valid);

	return base_verification && maxima_valid;
}

template <class Node, class NodeTraits, class Options, class Tag>
bool
IntervalTree<Node, NodeTraits, Options, Tag>::verify_maxima(Node * n) const
{
	bool valid = true;
	auto maximum = NodeTraits::get_upper(*n);

	if (n->get_right() != nullptr) {
		maximum = std::max(maximum, n->get_right()->INB::_it_max_upper);
		valid &= this->verify_maxima(n->get_right());
	}
	if (n->get_left() != nullptr) {
		maximum = std::max(maximum, n->get_left()->INB::_it_max_upper);
		valid &= this->verify_maxima(n->get_left());
	}

	valid &= (maximum == n->INB::_it_max_upper);

	return valid;
}

template <class Node, class NodeTraits, class Options, class Tag>
void
IntervalTree<Node, NodeTraits, Options, Tag>::fixup_maxima(Node & node)
{
	ENodeTraits::fix_node(node);
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class Comparable>
typename IntervalTree<Node, NodeTraits, Options,
                      Tag>::template QueryResult<Comparable>
IntervalTree<Node, NodeTraits, Options, Tag>::query(const Comparable & q) const
{
	Node * cur = this->root;
	if (this->root == nullptr) {
		return QueryResult<Comparable>(nullptr, q);
	}

	while ((cur->get_left() != nullptr) &&
	       (cur->get_left()->INB::_it_max_upper >= NodeTraits::get_lower(q))) {
		cur = cur->get_left();
	}

	Node * hit;
	// If this overlaps, this is our first hit. otherwise, find the next one
	if ((NodeTraits::get_lower(q) <= NodeTraits::get_upper(*cur)) &&
	    (NodeTraits::get_upper(q) >= NodeTraits::get_lower(*cur))) {
		hit = cur;
	} else {
		hit =
		    intervaltree_internal::find_next_overlapping<Node, INB, NodeTraits,
		                                                 false, Comparable>(cur, q);
	}

	return QueryResult<Comparable>(hit, q);
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class Comparable>
typename IntervalTree<Node, NodeTraits, Options,
                      Tag>::BaseTree::template iterator<false>
IntervalTree<Node, NodeTraits, Options, Tag>::find(const Comparable & q)
{
	// dispatch based on whether intervals are also sorted by upper bound
	if (Options::itree_fast_find) {
		return this->find_fast(q);
	} else {
		return this->find_slow(q);
	}
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class Comparable>
typename IntervalTree<Node, NodeTraits, Options,
                      Tag>::BaseTree::template const_iterator<false>
IntervalTree<Node, NodeTraits, Options, Tag>::find(const Comparable & q) const
{
	return const_cast<std::remove_const_t<decltype(this)>>(this)->contains(q);
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class Comparable>
typename IntervalTree<Node, NodeTraits, Options,
                      Tag>::BaseTree::template iterator<false>
IntervalTree<Node, NodeTraits, Options, Tag>::find_fast(const Comparable & q)
{
	Node * cur = this->root;
	Node * last_left = nullptr;

	const auto & q_lower = NodeTraits::get_lower(q);
	const auto & q_upper = NodeTraits::get_upper(q);

	// Step 1: Search on lower bounds
	while (cur != nullptr) {
		if constexpr (Options::micro_prefetch) {
			__builtin_prefetch(cur->INB::get_left());
			__builtin_prefetch(cur->INB::get_right());
		}

		if constexpr (Options::micro_avoid_conditionals) {
			(void)last_left;

			if (__builtin_expect(q_lower == NodeTraits::get_lower(*cur), false)) {
				// Found a node with the correct lower bound. All nodes with the same
				// lower bound must live in this subtree. Start the upper-bound search
				// here.
				break;
			}
			cur = utilities::go_right_if(NodeTraits::get_lower(*cur) < q_lower, cur);
		} else {
			// Two-way search with the last_left-trick, analogous to find() in bst.cpp
			if (NodeTraits::get_lower(*cur) < q_lower) {
				cur = cur->INB::get_right();
			} else {
				last_left = cur;
				cur = cur->INB::get_left();
			}
		}
	}

	// If we use the last_left trick, we need to check if we found
	// a match the last time we went left
	if constexpr (!Options::micro_avoid_conditionals) {
		if ((last_left != nullptr) &&
		    (q_lower == NodeTraits::get_lower(*last_left))) {
			// Yep, found! Set cur to continue upper-search from there
			cur = last_left;
		}
	}

	// Step 2: Search on upper bounds. Essentially the same as before, but we must
	// make sure never to leave the range of nodes with correct lower bounds.
	while ((cur != nullptr) && (NodeTraits::get_lower(*cur) == q_lower)) {
		if constexpr (Options::micro_prefetch) {
			__builtin_prefetch(cur->INB::get_left());
			__builtin_prefetch(cur->INB::get_right());
		}

		if constexpr (Options::micro_avoid_conditionals) {
			(void)last_left;

			if (__builtin_expect(q_upper == NodeTraits::get_upper(*cur), false)) {
				// Upper and lower bound check out
				return typename BaseTree::template iterator<false>(*cur);
			}
			cur = utilities::go_right_if(NodeTraits::get_upper(*cur) < q_upper, cur);
		} else {
			// Two-way search with the last_left-trick, analogous to find() in bst.cpp
			if (NodeTraits::get_upper(*cur) < q_upper) {
				cur = cur->INB::get_right();
			} else {
				last_left = cur;
				cur = cur->INB::get_left();
			}
		}
	}

	if constexpr (!Options::micro_avoid_conditionals) {
		if ((last_left != nullptr) &&
		    (q_upper == NodeTraits::get_upper(*last_left))) {
			return typename BaseTree::template iterator<false>(last_left);
		}
	}

	return this->end();
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class Comparable>
typename IntervalTree<Node, NodeTraits, Options,
                      Tag>::BaseTree::template iterator<false>
IntervalTree<Node, NodeTraits, Options, Tag>::find_slow(const Comparable & q)
{
	const auto & q_lower = NodeTraits::get_lower(q);
	const auto & q_upper = NodeTraits::get_upper(q);

	// TODO remove the const-away-cast below once non-const
	// queries are implemented
	for (auto & hit : this->query(q)) {
		if ((NodeTraits::get_lower(hit) == q_lower) &&
		    (NodeTraits::get_upper(hit) == q_upper)) {
			return
			    typename BaseTree::template iterator<false>(const_cast<Node *>(&hit));
		}
	}

	return this->end();
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class Comparable>
typename IntervalTree<Node, NodeTraits, Options,
                      Tag>::BaseTree::template const_iterator<false>
IntervalTree<Node, NodeTraits, Options, Tag>::interval_upper_bound(
    const Comparable & query_range) const
{
	// An interval lying strictly after <query> is an upper-bound (in the RBTree
	// sense) of the interval that just spans the last point of <query>
	intervaltree_internal::DummyRange<typename NodeTraits::key_type> dummy_range(
	    NodeTraits::get_upper(query_range), NodeTraits::get_upper(query_range));

	return this->upper_bound(dummy_range);
}

// TODO move stuff here
namespace intervaltree_internal {

template <class KeyType>
DummyRange<KeyType>::DummyRange(KeyType lower, KeyType upper)
    : std::pair<KeyType, KeyType>(lower, upper)
{}

template <class Node, class INB, class NodeTraits, bool skipfirst,
          class Comparable>
Node *
find_next_overlapping(Node * cur, const Comparable & q)
{
	// We search for the next bigger node, pruning the search as necessary. When
	// Pruning occurs, we need to restart the search for the next larger node.

	do {
		// We make sure that at the start of the loop, the lower of cur is smaller
		// than the upper of q. Thus, we need to only check the upper to check for
		// overlap.
		if (cur->get_right() != nullptr) {
			// go to smallest larger-or-equal child
			cur = cur->get_right();
			if (cur->INB::_it_max_upper < NodeTraits::get_lower(q)) {
				// Prune!
				// Nothing starting from this node can overlap b/c of upper limit.
				// Backtrack.
				while ((cur->get_parent() != nullptr) &&
				       (cur->get_parent()->get_right() ==
				        cur)) { // these are the nodes which are smaller and were
					              // already visited
					cur = cur->get_parent();
				}

				// go one further up
				if (cur->get_parent() == nullptr) {
					// Backtracked out of the root
					return nullptr;
				} else {
					// go up
					cur = cur->get_parent();
				}
			} else {
				while (cur->get_left() != nullptr) {
					cur = cur->get_left();
					if (cur->INB::_it_max_upper < NodeTraits::get_lower(q)) {
						// Prune!
						// Nothing starting from this node can overlap. Backtrack.
						cur = cur->get_parent();
						break;
					}
				}
			}
		} else {
			// go up
			// skip over the nodes already visited
			while ((cur->get_parent() != nullptr) &&
			       (cur->get_parent()->get_right() ==
			        cur)) { // these are the nodes which are smaller and were already
				              // visited
				cur = cur->get_parent();
			}

			// go one further up
			if (cur->get_parent() == nullptr) {
				// Backtracked into the root
				return nullptr;
			} else {
				// go up
				cur = cur->get_parent();
			}
		}

		if (NodeTraits::get_lower(*cur) > NodeTraits::get_upper(q)) {
			// No larger node can be an overlap!
			return nullptr;
		}

		if (NodeTraits::get_upper(*cur) >= NodeTraits::get_lower(q)) {
			// Found!
			return cur;
		}

	} while (true);
}

} // namespace intervaltree_internal

template <class Node, class NodeTraits, class Options, class Tag>
template <class Comparable>
IntervalTree<Node, NodeTraits, Options,
             Tag>::QueryResult<Comparable>::QueryResult(Node * n_in,
                                                        const Comparable & q_in)
    : n(n_in), q(q_in)
{}

template <class Node, class NodeTraits, class Options, class Tag>
template <class Comparable>
typename IntervalTree<Node, NodeTraits, Options,
                      Tag>::template QueryResult<Comparable>::const_iterator
IntervalTree<Node, NodeTraits, Options, Tag>::QueryResult<Comparable>::begin()
    const
{
	return const_iterator(this->n, this->q);
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class Comparable>
typename IntervalTree<Node, NodeTraits, Options,
                      Tag>::template QueryResult<Comparable>::const_iterator
IntervalTree<Node, NodeTraits, Options, Tag>::QueryResult<Comparable>::end()
    const
{
	return const_iterator(nullptr, this->q);
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class Comparable>
IntervalTree<Node, NodeTraits, Options, Tag>::QueryResult<
    Comparable>::const_iterator::const_iterator(Node * n_in,
                                                const Comparable & q_in)
    : n(n_in), q(q_in)
{}

template <class Node, class NodeTraits, class Options, class Tag>
template <class Comparable>
IntervalTree<Node, NodeTraits, Options, Tag>::QueryResult<Comparable>::
    const_iterator::const_iterator(
        const typename IntervalTree<Node, NodeTraits, Options, Tag>::
            template QueryResult<Comparable>::const_iterator & other)
    : n(other.n), q(other.q)
{}

template <class Node, class NodeTraits, class Options, class Tag>
template <class Comparable>
IntervalTree<Node, NodeTraits, Options,
             Tag>::QueryResult<Comparable>::const_iterator::~const_iterator()
{}

template <class Node, class NodeTraits, class Options, class Tag>
template <class Comparable>
typename IntervalTree<Node, NodeTraits, Options,
                      Tag>::template QueryResult<Comparable>::const_iterator &
IntervalTree<Node, NodeTraits, Options, Tag>::QueryResult<Comparable>::
    const_iterator::operator=(
        const typename IntervalTree<Node, NodeTraits, Options, Tag>::
            template QueryResult<Comparable>::const_iterator & other)
{
	this->n = other.n;
	this->q = other.q;
	return *this;
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class Comparable>
bool
IntervalTree<Node, NodeTraits, Options, Tag>::QueryResult<Comparable>::
    const_iterator::operator==(
        const typename IntervalTree<Node, NodeTraits, Options, Tag>::
            template QueryResult<Comparable>::const_iterator & other) const
{
	return ((this->n == other.n) &&
	        (NodeTraits::get_lower(this->q) == NodeTraits::get_lower(other.q)) &&
	        (NodeTraits::get_upper(this->q) == NodeTraits::get_upper(other.q)));
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class Comparable>
bool
IntervalTree<Node, NodeTraits, Options, Tag>::QueryResult<Comparable>::
    const_iterator::operator!=(
        const typename IntervalTree<Node, NodeTraits, Options, Tag>::
            template QueryResult<Comparable>::const_iterator & other) const
{
	return !(*this == other);
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class Comparable>
typename IntervalTree<Node, NodeTraits, Options,
                      Tag>::template QueryResult<Comparable>::const_iterator &
IntervalTree<Node, NodeTraits, Options,
             Tag>::QueryResult<Comparable>::const_iterator::operator++()
{
	this->n = intervaltree_internal::find_next_overlapping<Node, INB, NodeTraits,
	                                                       false, Comparable>(
	    this->n, this->q);

	return *this;
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class Comparable>
typename IntervalTree<Node, NodeTraits, Options,
                      Tag>::template QueryResult<Comparable>::const_iterator
IntervalTree<Node, NodeTraits, Options,
             Tag>::QueryResult<Comparable>::const_iterator::operator++(int)
{
	typename IntervalTree<Node, NodeTraits, Options, Tag>::template QueryResult<
	    Comparable>::const_iterator cpy(*this);

	this->operator++();

	return cpy;
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class Comparable>
const Node &
IntervalTree<Node, NodeTraits, Options,
             Tag>::QueryResult<Comparable>::const_iterator::operator*() const
{
	return *(this->n);
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class Comparable>
const Node *
IntervalTree<Node, NodeTraits, Options,
             Tag>::QueryResult<Comparable>::const_iterator::operator->() const
{
	return this->n;
}

template <class Node, class NodeTraits, class Options, class Tag>
void
IntervalTree<Node, NodeTraits, Options, Tag>::dump_to_dot(
    const std::string & filename) const
{
	this->dump_to_dot_base(filename, [&](const Node * node) {
		return NodeTraits::get_id(node) + std::string("\n[") +
		       std::to_string(NodeTraits::get_lower(*node)) + std::string(", ") +
		       std::to_string(NodeTraits::get_upper(*node)) + std::string("]\n") +
		       std::string("-> ") + std::to_string(node->INB::_it_max_upper);
	});
}

} // namespace ygg
