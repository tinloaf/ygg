//
// Created by lukas on 18.09.17.
//

#include "list.hpp"

namespace ygg {

template <class Node, class Options, class Tag>
List<Node, Options, Tag>::List() : head(nullptr), tail(nullptr)
{}

template <class Node, class Options, class Tag>
template <class ConcreteIterator, class BaseType>
List<Node, Options, Tag>::IteratorBase<ConcreteIterator,
                                       BaseType>::IteratorBase()
    : n(nullptr), l(nullptr)
{}

template <class Node, class Options, class Tag>
template <class ConcreteIterator, class BaseType>
List<Node, Options, Tag>::IteratorBase<
    ConcreteIterator, BaseType>::IteratorBase(List<Node, Options, Tag> * l_in,
                                              BaseType * n_in)
    : n(n_in), l(l_in)
{}

template <class Node, class Options, class Tag>
template <class ConcreteIterator, class BaseType>
List<Node, Options, Tag>::IteratorBase<
    ConcreteIterator, BaseType>::IteratorBase(const ConcreteIterator & other)
    : n(other.n), l(other.l)
{}

template <class Node, class Options, class Tag>
template <class ConcreteIterator, class BaseType>
ConcreteIterator &
List<Node, Options, Tag>::IteratorBase<ConcreteIterator, BaseType>::
operator=(const ConcreteIterator & other)
{
  this->n = other.n;
  this->l = other.l;

  return *(static_cast<ConcreteIterator *>(this));
}

template <class Node, class Options, class Tag>
template <class ConcreteIterator, class BaseType>
ConcreteIterator &
List<Node, Options, Tag>::IteratorBase<ConcreteIterator, BaseType>::
operator=(ConcreteIterator && other)
{
  this->n = other.n;
  this->l = other.l;

  return *(static_cast<ConcreteIterator *>(this));
}

template <class Node, class Options, class Tag>
template <class ConcreteIterator, class BaseType>
bool
List<Node, Options, Tag>::IteratorBase<ConcreteIterator, BaseType>::
operator==(const ConcreteIterator & other) const
{
  return (this->n == other.n) && (this->l == other.l);
}

template <class Node, class Options, class Tag>
template <class ConcreteIterator, class BaseType>
bool
List<Node, Options, Tag>::IteratorBase<ConcreteIterator, BaseType>::
operator!=(const ConcreteIterator & other) const
{
  return (this->n != other.n) || (this->l != other.l);
}

template <class Node, class Options, class Tag>
template <class ConcreteIterator, class BaseType>
ConcreteIterator &
List<Node, Options, Tag>::IteratorBase<ConcreteIterator, BaseType>::operator++()
{
  this->n = this->n->NB::_l_next;

  return (*(static_cast<ConcreteIterator *>(this)));
}

template <class Node, class Options, class Tag>
template <class ConcreteIterator, class BaseType>
ConcreteIterator
List<Node, Options, Tag>::IteratorBase<ConcreteIterator, BaseType>::
operator++(int)
{
  ConcreteIterator cpy(*(static_cast<ConcreteIterator *>(this)));
  this->operator++();
  return cpy;
}

template <class Node, class Options, class Tag>
template <class ConcreteIterator, class BaseType>
ConcreteIterator &
List<Node, Options, Tag>::IteratorBase<ConcreteIterator, BaseType>::
operator+=(size_t steps)
{
  for (size_t i = 0; i < steps; ++i) {
    this->operator++();
  }

  return (*(static_cast<ConcreteIterator *>(this)));
}

template <class Node, class Options, class Tag>
template <class ConcreteIterator, class BaseType>
ConcreteIterator &
List<Node, Options, Tag>::IteratorBase<ConcreteIterator, BaseType>::
operator-=(size_t steps)
{
  for (size_t i = 0; i < steps; ++i) {
    this->operator--();
  }

  return (*(static_cast<ConcreteIterator *>(this)));
}

template <class Node, class Options, class Tag>
template <class ConcreteIterator, class BaseType>
ConcreteIterator
List<Node, Options, Tag>::IteratorBase<ConcreteIterator, BaseType>::
operator+(size_t steps) const
{
  ConcreteIterator cpy(*(static_cast<const ConcreteIterator *>(this)));
  cpy += steps;
  return cpy;
}

template <class Node, class Options, class Tag>
template <class ConcreteIterator, class BaseType>
ConcreteIterator
List<Node, Options, Tag>::IteratorBase<ConcreteIterator, BaseType>::
operator-(size_t steps) const
{
  ConcreteIterator cpy(*(static_cast<const ConcreteIterator *>(this)));
  cpy -= steps;
  return cpy;
}

template <class Node, class Options, class Tag>
template <class ConcreteIterator, class BaseType>
ConcreteIterator &
List<Node, Options, Tag>::IteratorBase<ConcreteIterator, BaseType>::operator--()
{
  this->n = this->n->NB::_l_prev;

  return (*(static_cast<ConcreteIterator *>(this)));
}

template <class Node, class Options, class Tag>
template <class ConcreteIterator, class BaseType>
ConcreteIterator
List<Node, Options, Tag>::IteratorBase<ConcreteIterator, BaseType>::
operator--(int)
{
  ConcreteIterator cpy(*(static_cast<ConcreteIterator *>(this)));
  this->operator--();
  return cpy;
}

template <class Node, class Options, class Tag>
template <class ConcreteIterator, class BaseType>
typename List<Node, Options, Tag>::template IteratorBase<ConcreteIterator,
                                                         BaseType>::reference
    List<Node, Options, Tag>::IteratorBase<ConcreteIterator, BaseType>::
    operator*() const
{
  return *(this->n);
}

template <class Node, class Options, class Tag>
template <class ConcreteIterator, class BaseType>
typename List<Node, Options, Tag>::template IteratorBase<ConcreteIterator,
                                                         BaseType>::pointer
    List<Node, Options, Tag>::IteratorBase<ConcreteIterator, BaseType>::
    operator->() const
{
  return this->n;
}

template <class Node, class Options, class Tag>
void
List<Node, Options, Tag>::insert(Node * next, Node * n)
{
  this->s.add(1);

  Node * prev;
  if (next != nullptr) {
    prev = next->NB::_l_prev;
    next->NB::_l_prev = n;
  } else {
    prev = this->tail;
    this->tail = n;
  }

  if (prev != nullptr) {
    prev->NB::_l_next = n;
  } else {
    this->head = n;
  }

  n->NB::_l_next = next;
  n->NB::_l_prev = prev;
}

template <class Node, class Options, class Tag>
void
List<Node, Options, Tag>::remove(Node * n)
{
  this->s.reduce(1);

  if (n->NB::_l_next != nullptr) {
    n->NB::_l_next->NB::_l_prev = n->NB::_l_prev;
  } else {
    this->tail = n->NB::_l_prev;
  }

  if (n->NB::_l_prev != nullptr) {
    n->NB::_l_prev->NB::_l_next = n->NB::_l_next;
  } else {
    this->head = n->NB::_l_next;
  }
}

template <class Node, class Options, class Tag>
typename List<Node, Options, Tag>::iterator
List<Node, Options, Tag>::begin()
{
  return iterator(this, this->head);
}

template <class Node, class Options, class Tag>
typename List<Node, Options, Tag>::const_iterator
List<Node, Options, Tag>::begin() const
{
  return const_iterator(this, this->head);
}

template <class Node, class Options, class Tag>
typename List<Node, Options, Tag>::iterator
List<Node, Options, Tag>::back()
{
  return iterator(this, this->tail);
}

template <class Node, class Options, class Tag>
typename List<Node, Options, Tag>::const_iterator
List<Node, Options, Tag>::back() const
{
  return const_iterator(this, this->tail);
}

template <class Node, class Options, class Tag>
typename List<Node, Options, Tag>::iterator
List<Node, Options, Tag>::end()
{
  return iterator(this, nullptr);
}

template <class Node, class Options, class Tag>
typename List<Node, Options, Tag>::const_iterator
List<Node, Options, Tag>::end() const
{
  return const_iterator(this, nullptr);
}

template <class Node, class Options, class Tag>
typename List<Node, Options, Tag>::const_iterator
List<Node, Options, Tag>::iterator_to(const Node & n) const
{
  return const_iterator(const_cast<List<Node, Tag> *>(this), &n);
}

template <class Node, class Options, class Tag>
typename List<Node, Options, Tag>::iterator
List<Node, Options, Tag>::iterator_to(const Node & n)
{
  return iterator(this, const_cast<Node *>(&n));
}

template <class Node, class Options, class Tag>
size_t
List<Node, Options, Tag>::size() const
{
  return this->s.get();
}

template <class Node, class Options, class Tag>
bool
List<Node, Options, Tag>::empty() const
{
  return this->head == nullptr;
}

template <class Node, class Options, class Tag>
void
List<Node, Options, Tag>::clear()
{
  this->head = nullptr;
  this->tail = nullptr;
  this->s.set(0);
}

} // namespace ygg