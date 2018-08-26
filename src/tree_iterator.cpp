#include <cstddef>

#include "tree_iterator.hpp"

namespace ygg {
namespace internal {

template <class ConcreteIterator, class Node, class NodeInterface, bool reverse>
void
IteratorBase<ConcreteIterator, Node, NodeInterface, reverse>::step_forward()
{
  // No more equal elements
  if (NodeInterface::get_right(this->n) != nullptr) {
    // go to smallest larger-or-equal child
    this->n = NodeInterface::get_right(this->n);
    while (NodeInterface::get_left(this->n) != nullptr) {
      this->n = NodeInterface::get_left(this->n);
    }
  } else {
    // go up

    // skip over the nodes already visited
    while ((NodeInterface::get_parent(this->n) != nullptr) &&
           (NodeInterface::get_right(NodeInterface::get_parent(this->n)) ==
            this->n)) { // these are the nodes which are smaller and were
                        // already visited
      this->n = NodeInterface::get_parent(this->n);
    }

    // go one further up
    if (NodeInterface::get_parent(this->n) == nullptr) {
      // done
      this->n = nullptr;
    } else {
      // go up
      this->n = NodeInterface::get_parent(this->n);
    }
  }
}

template <class ConcreteIterator, class Node, class NodeInterface, bool reverse>
void
IteratorBase<ConcreteIterator, Node, NodeInterface, reverse>::step_back()
{
  if (NodeInterface::get_left(this->n) != nullptr) {
    // go to largest smaller child
    this->n = NodeInterface::get_left(this->n);
    while (NodeInterface::get_right(this->n) != nullptr) {
      this->n = NodeInterface::get_right(this->n);
    }
  } else {
    // go up

    // skip over the nodes already visited
    while ((NodeInterface::get_parent(this->n) != nullptr) &&
           (NodeInterface::get_left(NodeInterface::get_parent(this->n)) ==
            this->n)) { // these are the nodes which are larger and were already
                        // visited
      this->n = NodeInterface::get_parent(this->n);
    }

    // go one further up
    if (NodeInterface::get_parent(this->n) == nullptr) {
      // done
      this->n = nullptr;
    } else {
      // go up
      this->n = NodeInterface::get_parent(this->n);
    }
  }
}

template <class ConcreteIterator, class Node, class NodeInterface, bool reverse>
IteratorBase<ConcreteIterator, Node, NodeInterface, reverse>::IteratorBase()
    : n(nullptr)
{}

template <class ConcreteIterator, class Node, class NodeInterface, bool reverse>
IteratorBase<ConcreteIterator, Node, NodeInterface, reverse>::IteratorBase(
    Node * n_in)
    : n(n_in)
{}

template <class ConcreteIterator, class Node, class NodeInterface, bool reverse>
IteratorBase<ConcreteIterator, Node, NodeInterface, reverse>::IteratorBase(
    const ConcreteIterator & other)
    : n(other.n)
{}

template <class ConcreteIterator, class Node, class NodeInterface, bool reverse>
ConcreteIterator &
IteratorBase<ConcreteIterator, Node, NodeInterface, reverse>::
operator=(const ConcreteIterator & other)
{
  this->n = other.n;

  return *(static_cast<ConcreteIterator *>(this));
}

template <class ConcreteIterator, class Node, class NodeInterface, bool reverse>
ConcreteIterator &
IteratorBase<ConcreteIterator, Node, NodeInterface, reverse>::
operator=(ConcreteIterator && other)
{
  this->n = other.n;

  return *(static_cast<ConcreteIterator *>(this));
}

template <class ConcreteIterator, class Node, class NodeInterface, bool reverse>
bool
IteratorBase<ConcreteIterator, Node, NodeInterface, reverse>::
operator==(const ConcreteIterator & other) const
{
  return (this->n == other.n);
}

template <class ConcreteIterator, class Node, class NodeInterface, bool reverse>
bool
IteratorBase<ConcreteIterator, Node, NodeInterface, reverse>::
operator!=(const ConcreteIterator & other) const
{
  return (this->n != other.n);
}

template <class ConcreteIterator, class Node, class NodeInterface, bool reverse>
ConcreteIterator &
IteratorBase<ConcreteIterator, Node, NodeInterface, reverse>::operator++()
{
  this->dispatch_operator_pp();

  return (*(static_cast<ConcreteIterator *>(this)));
}

template <class ConcreteIterator, class Node, class NodeInterface, bool reverse>
ConcreteIterator &
IteratorBase<ConcreteIterator, Node, NodeInterface, reverse>::operator--()
{
  this->dispatch_operator_mm();

  return *(static_cast<ConcreteIterator *>(this));
}

template <class ConcreteIterator, class Node, class NodeInterface, bool reverse>
ConcreteIterator
IteratorBase<ConcreteIterator, Node, NodeInterface, reverse>::operator++(int)
{
  ConcreteIterator cpy(*(static_cast<ConcreteIterator *>(this)));
  this->operator++();
  return cpy;
}

template <class ConcreteIterator, class Node, class NodeInterface, bool reverse>
ConcreteIterator
IteratorBase<ConcreteIterator, Node, NodeInterface, reverse>::operator--(int)
{
  ConcreteIterator cpy(*(static_cast<ConcreteIterator *>(this)));
  this->operator--();
  return cpy;
}

template <class ConcreteIterator, class Node, class NodeInterface, bool reverse>
ConcreteIterator &
IteratorBase<ConcreteIterator, Node, NodeInterface, reverse>::
operator+=(size_t steps)
{
  for (size_t i = 0; i < steps; ++i) {
    this->operator++();
  }

  return (*(static_cast<ConcreteIterator *>(this)));
}

template <class ConcreteIterator, class Node, class NodeInterface, bool reverse>
ConcreteIterator
IteratorBase<ConcreteIterator, Node, NodeInterface, reverse>::
operator+(size_t steps) const
{
  ConcreteIterator cpy(this->n);
  cpy += steps;
  return cpy;
}

template <class ConcreteIterator, class Node, class NodeInterface, bool reverse>
ConcreteIterator &
IteratorBase<ConcreteIterator, Node, NodeInterface, reverse>::
operator-=(size_t steps)
{
  for (size_t i = 0; i < steps; ++i) {
    this->operator--();
  }

  return (*(static_cast<ConcreteIterator *>(this)));
}

template <class ConcreteIterator, class Node, class NodeInterface, bool reverse>
ConcreteIterator
IteratorBase<ConcreteIterator, Node, NodeInterface, reverse>::
operator-(size_t steps) const
{
  ConcreteIterator cpy(this->n);
  cpy -= steps;
  return cpy;
}

template <class ConcreteIterator, class Node, class NodeInterface, bool reverse>
typename IteratorBase<ConcreteIterator, Node, NodeInterface, reverse>::reference
    IteratorBase<ConcreteIterator, Node, NodeInterface, reverse>::
    operator*() const
{
  return *(this->n);
}

template <class ConcreteIterator, class Node, class NodeInterface, bool reverse>
typename IteratorBase<ConcreteIterator, Node, NodeInterface, reverse>::pointer
    IteratorBase<ConcreteIterator, Node, NodeInterface, reverse>::
    operator->() const
{
  return this->n;
}

} // namespace internal
} // namespace ygg
