template<class Node, class NodeTraits>
bool
IntervalCompare<Node, NodeTraits>::operator()(const Node & lhs, const Node & rhs) const
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

template<class Node, class NodeTraits>
void
ExtendedNodeTraits<Node, NodeTraits>::leaf_inserted(Node & node)
{
  node._it_max_upper = NodeTraits::get_upper(node);
  // TODO FIXME propagate up!
}

template<class Node, class NodeTraits>
void
ExtendedNodeTraits<Node, NodeTraits>::fix_node(Node & node)
{
  node._it_max_upper = std::max({NodeTraits::get_upper(node),
                                node._rbt_left->_it_max_upper,
                                node._rbt_right->_it_max_upper});
}

template<class Node, class NodeTraits>
void
ExtendedNodeTraits<Node, NodeTraits>::rotated_left(Node & node)
{
  // 'node' is the node that was the old parent.
  fix_node(node);
  fix_node(*(node._rbt_parent));
}

template<class Node, class NodeTraits>
void
ExtendedNodeTraits<Node, NodeTraits>::rotated_right(Node & node)
{
  // 'node' is the node that was the old parent.
  fix_node(node);
  fix_node(*(node._rbt_parent));
}

template<class Node, class NodeTraits>
void
ExtendedNodeTraits<Node, NodeTraits>::deleted_below(Node & node) {
  fix_node(node);
}

template<class Node, class NodeTraits>
void
ExtendedNodeTraits<Node, NodeTraits>::swapped(Node & n1, Node & n2) {
  fix_node(n1);
  if (n1._rbt_parent != nullptr) {
    fix_node(*(n1._rbt_parent));
  }

  fix_node(n2);
  if (n2._rbt_parent != nullptr) {
    fix_node(*(n2._rbt_parent));
  }
}

template<class Node, class NodeTraits>
IntervalTree<Node, NodeTraits>::IntervalTree()
{}

template<class Node, class NodeTraits>
bool
IntervalTree<Node, NodeTraits>::verify_integrity() const
{
  bool base_verification = this->BaseTree::verify_integrity();

  bool maxima_valid = this->verify_maxima(this->root);

  return base_verification && maxima_valid;
}

template<class Node, class NodeTraits>
bool
IntervalTree<Node, NodeTraits>::verify_maxima(Node * n) const
{
  bool valid = true;
  auto maximum = NodeTraits::get_upper(*n);

  if (n->_rbt_right != nullptr) {
    maximum = std::max(maximum, n->_rbt_right->_it_max_upper);
    valid &= this->verify_maxima(n->_rbt_right);
  }
  if (n->_rbt_left != nullptr) {
    maximum = std::max(maximum, n->_rbt_left->_it_max_upper);
    valid &= this->verify_maxima(n->_rbt_left);
  }

  valid &= (maximum == n->_it_max_upper);

  return valid;
}

template<class Node, class NodeTraits>
template<class Comparable>
IntervalTree<Node, NodeTraits>::QueryResult<Comparable>
IntervalTree<Node, NodeTraits>::query(const Comparable & q) const
{
  Node * cur;
  Node * start = nullptr;

  while ((cur->_rbt_left != nullptr) && (cur->_rbt_left->_it_max_upper >= NodeTraits::get_lower(q))) {
      cur = cur->_rbt_left;
  }

  // Everthing left of here ends too early. Find the next larger one.
  return QueryResult<Comparable>(
                     iitree::utilities::find_next_overlapping<Node, NodeTraits, false, Comparable>(cur, q),
                     q);
}

namespace iitree { namespace utilities {

template<class Node, class NodeTraits, bool skipfirst, class Comparable>
Node *
find_next_overlapping(Node * cur, const Comparable & q)
{
  // We search for the next bigger node, pruning the search as necessary. When
  // Pruning occurrs, we need to restart the search for the next larger node.

  do {
    // We make sure that at the start of the loop, the lower of cur is smaller
    // than the upper of q. Thus, we need to only check the upper to check for
    // overlap.

    if (!skipfirst) {
      if (NodeTraits::get_upper(*cur) >= NodeTraits::get_lower(q)) {
        // Found!
        return cur;
      }

      if (NodeTraits::get_lower(*cur) > NodeTraits::get_upper(q)) {
        // No larger node can be an overlap!
        return nullptr;
      }
    }

    if (cur->_rbt_right != nullptr) {
      // go to smallest larger-or-equal child
      cur = cur->_rbt_right;
      if (cur->_it_max_upper < NodeTraits::get_lower(q)) {
        // Prune!
        // Nothing starting from this node can overlap b/c of upper limit. Backtrack.
        while ((cur->_rbt_parent != nullptr) && (cur->_rbt_parent->_rbt_right == cur)) { // these are the nodes which are smaller and were already visited
          cur = cur->_rbt_parent;
        }

        // go one further up
        if (cur->_rbt_parent == nullptr) {
          return nullptr;
        } else {
          // go up
          cur = cur->_rbt_parent;
        }
      } else {
        while (cur->_rbt_left != nullptr) {
          cur = cur->_rbt_left;
          if (cur->_it_max_upper < NodeTraits::get_lower(q)) {
            // Prune!
            // Nothing starting from this node can overlap. Backtrack.
            cur = cur->_rbt_left;
            break;
          }
        }
      }
    } else {
      // go up

      // skip over the nodes already visited
      while ((cur->_rbt_parent != nullptr) && (cur->_rbt_parent->_rbt_right == cur)) { // these are the nodes which are smaller and were already visited
        cur = cur->_rbt_parent;
      }

      // go one further up
      if (cur->_rbt_parent == nullptr) {
        return nullptr;
      } else {
        // go up
        cur = cur->_rbt_parent;
      }
    }
  } while (cur != nullptr);

  // Nothing found? :(
  return nullptr;
}

} // namespace utilities
} // namespace iitree

template<class Node, class NodeTraits>
template<class Comparable>
IntervalTree<Node, NodeTraits>::QueryResult<Comparable>::QueryResult(Node * n_in, const Comparable & q_in)
  : n(n_in), q(q_in)
{}

template<class Node, class NodeTraits>
template<class Comparable>
typename IntervalTree<Node, NodeTraits>::template QueryResult<Comparable>::const_iterator
IntervalTree<Node, NodeTraits>::QueryResult<Comparable>::begin() const
{
  return const_iterator(this->n, this->q);
}

template<class Node, class NodeTraits>
template<class Comparable>
typename IntervalTree<Node, NodeTraits>::template QueryResult<Comparable>::const_iterator
IntervalTree<Node, NodeTraits>::QueryResult<Comparable>::end() const
{
  return const_iterator(nullptr, this->q);
}

template<class Node, class NodeTraits>
template<class Comparable>
IntervalTree<Node, NodeTraits>::QueryResult<Comparable>::const_iterator::const_iterator(Node * n_in, const Comparable & q_in)
  : n(n_in), q(q_in)
{}

template<class Node, class NodeTraits>
template<class Comparable>
IntervalTree<Node, NodeTraits>::QueryResult<Comparable>::const_iterator::const_iterator(const typename IntervalTree<Node, NodeTraits>::template QueryResult<Comparable>::const_iterator & other)
  : n(other.n), q(other.q)
{}

template<class Node, class NodeTraits>
template<class Comparable>
IntervalTree<Node, NodeTraits>::QueryResult<Comparable>::const_iterator::~const_iterator()
{}

template<class Node, class NodeTraits>
template<class Comparable>
typename IntervalTree<Node, NodeTraits>::template QueryResult<Comparable>::const_iterator &
IntervalTree<Node, NodeTraits>::QueryResult<Comparable>::const_iterator::operator=(const typename IntervalTree<Node, NodeTraits>::template QueryResult<Comparable>::const_iterator & other)
{
  this->n = other.n;
  this->upper_limit = other.upper_limit;
  return *this;
}

template<class Node, class NodeTraits>
template<class Comparable>
bool
IntervalTree<Node, NodeTraits>::QueryResult<Comparable>::const_iterator::operator==(const typename IntervalTree<Node, NodeTraits>::template QueryResult<Comparable>::const_iterator & other) const
{
  return (this->n == other.n) && (this->upper_limit == other.upper_limit);
}

template<class Node, class NodeTraits>
template<class Comparable>
typename IntervalTree<Node, NodeTraits>::template QueryResult<Comparable>::const_iterator &
IntervalTree<Node, NodeTraits>::QueryResult<Comparable>::const_iterator::operator++()
{
  this->n = iitree::utilities::find_next_overlapping(this->n, this->q);
  return *this;
}

template<class Node, class NodeTraits>
template<class Comparable>
const Node &
IntervalTree<Node, NodeTraits>::QueryResult<Comparable>::const_iterator::operator*() const
{
  return *(this->cur);
}

template<class Node, class NodeTraits>
template<class Comparable>
const Node *
IntervalTree<Node, NodeTraits>::QueryResult<Comparable>::const_iterator::operator->() const
{
  return this->cur;
}
