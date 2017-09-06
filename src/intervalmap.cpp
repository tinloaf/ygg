//
// Created by lukas on 04.09.17.
//

#include "intervalmap.hpp"

namespace ygg {

template <class Node, class NodeTraits, int Tag>
void
IntervalMap<Node, NodeTraits, Tag>::insert(Node &n)
{
	// TODO this is redundant.
	n._imap_begin.point = NodeTraits::get_lower(n);
	n._imap_end.point = NodeTraits::get_upper(n);

	this->t.insert(n._imap_begin);
	this->t.insert(n._imap_end);

	// Determine the aggregate from before the begin
	auto begin_it = this->t.iterator_to(n._imap_begin);
	if (++begin_it != this->t.iterator_to(n._imap_end) &&
					begin_it->point == NodeTraits::get_lower(n)) {
		n._imap_begin.aggregate = begin_it->aggregate;
	} else {
		begin_it--;
		if (begin_it != this->t.begin()) {
			begin_it--;
			n._imap_begin.aggregate = begin_it->aggregate;
		}
	}

	// Set the aggregate after the end point
	/*
	auto end_it = this->t.iterator_to(n._imap_end);
	if ((end_it + 1) != this->t.end() && (end_it + 1)->point == NodeTraits::get_upper(n)) {
		// there are multiple events here, we're not the last.
		++end_it;
		n._imap_end.aggregate = end_it->aggregate;
	} else {
		--end_it;
		n._imap_end.aggregate = end_it->aggregate;
		end_it++;
	}
	 */
	auto end_it = this->t.iterator_to(n._imap_end);
	--end_it;
	n._imap_end.aggregate = end_it->aggregate;

	auto it = this->t.find(n._imap_begin.point); // guaranteed to be the first (in tree order)
																			         // section with this point

	while (it->point < NodeTraits::get_upper(n)) {
		value_type old_val = it->aggregate;
		it->aggregate += NodeTraits::get_value(n);

		NodeTraits::on_value_changed(*it, old_val, it->aggregate);

		it++;
	}
}

template <class Node, class NodeTraits, int Tag>
void
IntervalMap<Node, NodeTraits, Tag>::remove(Node &n)
{
  auto it = this->t.find(n._imap_begin.point);
	auto stop_it = this->t.find(n._imap_end.point);

	while (it != stop_it) {
		value_type old_val = it->aggregate;
		it->aggregate -= NodeTraits::get_value(n);
		NodeTraits::on_value_changed(*it, old_val, it->aggregate);
		it++;
	}

	this->t.remove(n._imap_begin);
	this->t.remove(n._imap_end);
}

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::IteratorBase()
	: t(nullptr)
{}

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::IteratorBase(
				const ConcreteIterator & other)
				: inner(other.inner), t(other.t)
{}

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::IteratorBase(
				const InnerIterator & it, ITree * t_in)
				: inner(it), t(t_in)
{
	if ((this->inner == this->t->end()) || ((this->inner + 1) == this->t->end())) {
		this->inner = this->t->end();
	}
}

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
ConcreteIterator &
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator=(
				const ConcreteIterator & other)
{
	this->inner = other.inner;
	this->t = other.t;
	return *this;
};

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
ConcreteIterator &
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator=(
				ConcreteIterator && other)
{
	this->inner = std::move(other.inner);
	this->t = other.t;
	return *this;
};

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
bool
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator==(
				const ConcreteIterator & other) const
{
	return other.inner == this->inner;
}

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
bool
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator!=(
				const ConcreteIterator & other) const
{
	return other.inner != this->inner;
}


template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
ConcreteIterator &
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator++()
{
	auto old_point = this->inner->point;
	while (this->inner->point == old_point) {
		++(this->inner);
	}

	if ((this->inner + 1) == this->t->end()) {
		this->inner = this->t->end();
	}

	return *(static_cast<ConcreteIterator *>(this));
}

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
ConcreteIterator
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator++(int)
{
	ConcreteIterator buf(*(static_cast<ConcreteIterator *>(this)));

	auto old_point = this->inner->point;
	while (this->inner->point == old_point) {
		++(this->inner);
	}

	if ((this->inner + 1) == this->t->end()) {
		this->inner = this->t->end();
	}

	return buf;
}

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
ConcreteIterator &
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator+=(size_t steps)
{
	this->inner += steps;

	if ((this->inner == this->t->end()) || ((this->inner + 1) == this->t->end())) {
		this->inner = this->t->end();
	}

	return *this;
};

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
ConcreteIterator
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator+(size_t steps) const
{
	iterator it(this->inner + steps, this->t);
	return it;
};

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
ConcreteIterator &
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator--()
{
	if (this->inner == this->t.end()) {
		--(this->inner);
	}
	--(this->inner);
	return *this;
};

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
ConcreteIterator
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator--(int)
{
	iterator buf(*this);

	if (this->inner == this->t.end()) {
		--(this->inner);
	}
	--(this->inner);

	return buf;
};

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
typename IntervalMap<Node, NodeTraits, Tag>::template IteratorBase<ConcreteIterator,
                                                                   InnerIterator>::reference
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator*() const
{
	return *(this->inner);
}

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
typename IntervalMap<Node, NodeTraits, Tag>::template IteratorBase<ConcreteIterator,
                                                                   InnerIterator>::pointer
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator->() const
{
	return this->inner->operator->();
}

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
typename IntervalMap<Node, NodeTraits, Tag>::key_type
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::get_lower() const
{
	return this->inner->point;
}

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
typename IntervalMap<Node, NodeTraits, Tag>::key_type
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::get_upper() const
{
	return (this->inner + 1)->point;
}

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
const typename IntervalMap<Node, NodeTraits, Tag>::value_type &
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::get_value() const
{
	return this->inner->aggregate;
}

template <class Node, class NodeTraits, int Tag>
typename IntervalMap<Node, NodeTraits, Tag>::const_iterator
IntervalMap<Node, NodeTraits, Tag>::begin() const
{
	return const_iterator(this->t.begin(), & this->t);
};

template <class Node, class NodeTraits, int Tag>
typename IntervalMap<Node, NodeTraits, Tag>::const_iterator
IntervalMap<Node, NodeTraits, Tag>::end() const
{
	return const_iterator(this->t.end(), & this->t);
};

template <class Node, class NodeTraits, int Tag>
typename IntervalMap<Node, NodeTraits, Tag>::iterator
IntervalMap<Node, NodeTraits, Tag>::begin()
{
	return iterator(this->t.begin(), & this->t);
};

template <class Node, class NodeTraits, int Tag>
typename IntervalMap<Node, NodeTraits, Tag>::iterator
IntervalMap<Node, NodeTraits, Tag>::end()
{
	return iterator(this->t.end(), & this->t);
};

} // namespace ygg