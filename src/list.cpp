//
// Created by lukas on 18.09.17.
//

#include "list.hpp"

namespace ygg {

template<class Node, class Tag>
List<Node, Tag>::List()
	: head(nullptr), tail(nullptr)
{}

template<class Node, class Tag>
template<class ConcreteIterator, class BaseType>
List<Node, Tag>::IteratorBase<ConcreteIterator, BaseType>::IteratorBase()
  : n(nullptr), l(nullptr)
{}

template<class Node, class Tag>
template<class ConcreteIterator, class BaseType>
List<Node, Tag>::IteratorBase<ConcreteIterator, BaseType>::IteratorBase(List *l_in, BaseType *n_in)
	: n(n_in), l(l_in)
{}

template<class Node, class Tag>
template<class ConcreteIterator, class BaseType>
List<Node, Tag>::IteratorBase<ConcreteIterator, BaseType>::IteratorBase(const ConcreteIterator &other)
	: n(other.n), l(other.l)
{}

template<class Node, class Tag>
template<class ConcreteIterator, class BaseType>
ConcreteIterator &
List<Node, Tag>::IteratorBase<ConcreteIterator, BaseType>::operator=(const ConcreteIterator & other)
{
	this->n = other.n;
	this->l = other.l;

	return *(static_cast<ConcreteIterator *>(this));
}

template<class Node, class Tag>
template<class ConcreteIterator, class BaseType>
ConcreteIterator &
List<Node, Tag>::IteratorBase<ConcreteIterator, BaseType>::operator=(ConcreteIterator && other)
{
	this->n = other.n;
	this->l = other.l;

	return *(static_cast<ConcreteIterator *>(this));
}

template<class Node, class Tag>
template<class ConcreteIterator, class BaseType>
bool
List<Node, Tag>::IteratorBase<ConcreteIterator, BaseType>::operator==(const ConcreteIterator & other) const
{
	return (this->n == other.n) && (this->l == other.l);
}

template<class Node, class Tag>
template<class ConcreteIterator, class BaseType>
bool
List<Node, Tag>::IteratorBase<ConcreteIterator, BaseType>::operator!=(const ConcreteIterator & other) const
{
	return (this->n != other.n) || (this->l != other.l);
}

template<class Node, class Tag>
template<class ConcreteIterator, class BaseType>
ConcreteIterator &
List<Node, Tag>::IteratorBase<ConcreteIterator, BaseType>::operator++()
{
	this->n = this->n->NB::_l_next;

	return (*(static_cast<ConcreteIterator *>(this)));
}

template<class Node, class Tag>
template<class ConcreteIterator, class BaseType>
ConcreteIterator
List<Node, Tag>::IteratorBase<ConcreteIterator, BaseType>::operator++(int)
{
	ConcreteIterator cpy(*(static_cast<ConcreteIterator *>(this)));
	this->operator++();
	return cpy;
}

template<class Node, class Tag>
template<class ConcreteIterator, class BaseType>
ConcreteIterator &
List<Node, Tag>::IteratorBase<ConcreteIterator, BaseType>::operator+=(size_t steps)
{
	for (size_t i = 0 ; i < steps ; ++i) {
		this->operator++();
	}

	return (*(static_cast<ConcreteIterator *>(this)));
}

template<class Node, class Tag>
template<class ConcreteIterator, class BaseType>
ConcreteIterator &
List<Node, Tag>::IteratorBase<ConcreteIterator, BaseType>::operator-=(size_t steps)
{
	for (size_t i = 0 ; i < steps ; ++i) {
		this->operator--();
	}

	return (*(static_cast<ConcreteIterator *>(this)));
}

template<class Node, class Tag>
template<class ConcreteIterator, class BaseType>
ConcreteIterator
List<Node, Tag>::IteratorBase<ConcreteIterator, BaseType>::operator+(size_t steps) const
{
	ConcreteIterator cpy(*(static_cast<const ConcreteIterator *>(this)));
	cpy += steps;
	return cpy;
}

template<class Node, class Tag>
template<class ConcreteIterator, class BaseType>
ConcreteIterator
List<Node, Tag>::IteratorBase<ConcreteIterator, BaseType>::operator-(size_t steps) const
{
	ConcreteIterator cpy(*(static_cast<const ConcreteIterator *>(this)));
	cpy -= steps;
	return cpy;
}

template<class Node, class Tag>
template<class ConcreteIterator, class BaseType>
ConcreteIterator &
List<Node, Tag>::IteratorBase<ConcreteIterator, BaseType>::operator--()
{
	this->n = this->n->NB::_l_prev;

	return (*(static_cast<ConcreteIterator *>(this)));
}

template<class Node, class Tag>
template<class ConcreteIterator, class BaseType>
ConcreteIterator
List<Node, Tag>::IteratorBase<ConcreteIterator, BaseType>::operator--(int)
{
	ConcreteIterator cpy(*(static_cast<ConcreteIterator *>(this)));
	this->operator--();
	return cpy;
}

template<class Node, class Tag>
template<class ConcreteIterator, class BaseType>
typename List<Node, Tag>::template IteratorBase<ConcreteIterator, BaseType>::reference
List<Node, Tag>::IteratorBase<ConcreteIterator, BaseType>::operator*() const
{
	return *(this->n);
}

template<class Node, class Tag>
template<class ConcreteIterator, class BaseType>
typename List<Node, Tag>::template IteratorBase<ConcreteIterator, BaseType>::pointer
List<Node, Tag>::IteratorBase<ConcreteIterator, BaseType>::operator->() const
{
	return this->n;
}

template<class Node, class Tag>
void
List<Node, Tag>::insert(Node *next, Node *n)
{
	Node *prev;
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

template<class Node, class Tag>
void
List<Node, Tag>::remove(Node * n)
{
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

template<class Node, class Tag>
typename List<Node, Tag>::iterator
List<Node, Tag>::begin()
{
	return iterator(this, this->head);
}

template<class Node, class Tag>
typename List<Node, Tag>::const_iterator
List<Node, Tag>::begin() const
{
	return const_iterator(this, this->head);
}

template<class Node, class Tag>
typename List<Node, Tag>::iterator
List<Node, Tag>::back()
{
	return iterator(this, this->tail);
}

template<class Node, class Tag>
typename List<Node, Tag>::const_iterator
List<Node, Tag>::back() const
{
	return const_iterator(this, this->tail);
}

template<class Node, class Tag>
typename List<Node, Tag>::iterator
List<Node, Tag>::end()
{
	return iterator(this, nullptr);
}

template<class Node, class Tag>
typename List<Node, Tag>::const_iterator
List<Node, Tag>::end() const
{
	return const_iterator(this, nullptr);
}

template<class Node, class Tag>
typename List<Node, Tag>::const_iterator
List<Node, Tag>::iterator_to(const Node & n) const
{
	return const_iterator(const_cast<List<Node, Tag> *>(this), &n);
}

template<class Node, class Tag>
typename List<Node, Tag>::iterator
List<Node, Tag>::iterator_to(const Node & n)
{
	return iterator(this, const_cast<Node *>(&n));
}

} // namespace ygg