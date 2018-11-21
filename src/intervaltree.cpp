// Only for flymake / cquery
#include "intervaltree.hpp"

namespace ygg {
namespace intervaltree_internal {

template <class Node, class NodeTraits>
template <class T1, class T2>
bool
IntervalCompare<Node, NodeTraits>::operator()(const T1 & lhs,
                                              const T2 & rhs) const
{
	if (NodeTraits::get_lower(lhs) < NodeTraits::get_lower(rhs)) {
		return true;
	} else if ((NodeTraits::get_lower(lhs) == NodeTraits::get_lower(rhs)) &&
	           (NodeTraits::get_upper(lhs) < NodeTraits::get_upper(rhs))) {
		return true;
	} else {
		return false;
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

	if (node._rbt_left != nullptr) {
		node.INB::_it_max_upper =
		    std::max(node.INB::_it_max_upper, node._rbt_left->INB::_it_max_upper);
	}

	if (node._rbt_right != nullptr) {
		node.INB::_it_max_upper =
		    std::max(node.INB::_it_max_upper, node._rbt_right->INB::_it_max_upper);
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

	if (n->_rbt_right != nullptr) {
		maximum = std::max(maximum, n->_rbt_right->INB::_it_max_upper);
		valid &= this->verify_maxima(n->_rbt_right);
	}
	if (n->_rbt_left != nullptr) {
		maximum = std::max(maximum, n->_rbt_left->INB::_it_max_upper);
		valid &= this->verify_maxima(n->_rbt_left);
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

	while ((cur->_rbt_left != nullptr) &&
	       (cur->_rbt_left->INB::_it_max_upper >= NodeTraits::get_lower(q))) {
		cur = cur->_rbt_left;
	}
	// Everthing left of here ends too early.

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
	// Pruning occurrs, we need to restart the search for the next larger node.

	do {
		// We make sure that at the start of the loop, the lower of cur is smaller
		// than the upper of q. Thus, we need to only check the upper to check for
		// overlap.
		if (cur->_rbt_right != nullptr) {
			// go to smallest larger-or-equal child
			cur = cur->_rbt_right;
			if (cur->INB::_it_max_upper < NodeTraits::get_lower(q)) {
				// Prune!
				// Nothing starting from this node can overlap b/c of upper limit.
				// Backtrack.
				while ((cur->get_parent() != nullptr) &&
				       (cur->get_parent()->_rbt_right ==
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
				while (cur->_rbt_left != nullptr) {
					cur = cur->_rbt_left;
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
			       (cur->get_parent()->_rbt_right ==
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
IntervalTree<Node, NodeTraits, Options,
             Tag>::QueryResult<Comparable>::const_iterator::
operator=(const typename IntervalTree<Node, NodeTraits, Options, Tag>::
              template QueryResult<Comparable>::const_iterator & other)
{
	this->n = other.n;
	this->q = other.q;
	return *this;
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class Comparable>
bool
IntervalTree<Node, NodeTraits, Options,
             Tag>::QueryResult<Comparable>::const_iterator::
operator==(const typename IntervalTree<Node, NodeTraits, Options, Tag>::
               template QueryResult<Comparable>::const_iterator & other) const
{
	return ((this->n == other.n) &&
	        (NodeTraits::get_lower(this->q) == NodeTraits::get_lower(other.q)) &&
	        (NodeTraits::get_upper(this->q) && NodeTraits::get_upper(other.q)));
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class Comparable>
bool
IntervalTree<Node, NodeTraits, Options,
             Tag>::QueryResult<Comparable>::const_iterator::
operator!=(const typename IntervalTree<Node, NodeTraits, Options, Tag>::
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
const Node & IntervalTree<Node, NodeTraits, Options,
                          Tag>::QueryResult<Comparable>::const_iterator::
operator*() const
{
	return *(this->n);
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class Comparable>
const Node * IntervalTree<Node, NodeTraits, Options,
                          Tag>::QueryResult<Comparable>::const_iterator::
operator->() const
{
	return this->cur;
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
