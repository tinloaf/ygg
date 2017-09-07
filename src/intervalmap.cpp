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

	NodeTraits::on_segment_inserted(n._imap_begin);
	NodeTraits::on_segment_inserted(n._imap_end);

	// Determine the aggregate from before the begin
	auto begin_it = this->t.iterator_to(n._imap_begin);
	if (++begin_it != this->t.iterator_to(n._imap_end) &&
					begin_it->point == NodeTraits::get_lower(n)) {
		n._imap_begin.aggregate = begin_it->aggregate;
	} else {
		begin_it = this->t.iterator_to(n._imap_begin);
		if (begin_it != this->t.begin()) {
			begin_it--;
			n._imap_begin.aggregate = begin_it->aggregate;
			NodeTraits::on_length_changed(*begin_it);
		}
	}

	auto end_it = this->t.iterator_to(n._imap_end);
	--end_it;
	n._imap_end.aggregate = end_it->aggregate;
	if (end_it != this->t.iterator_to(n._imap_begin)) {
		NodeTraits::on_length_changed(*end_it);
	}

	auto it = this->t.find(n._imap_begin.point); // guaranteed to be the first (in tree order)
																			         // segment with this point

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

	bool notify_begin = false;
	auto begin_it = this->t.iterator_to(n._imap_begin);
	bool notify_end = false;
	auto end_it = this->t.iterator_to(n._imap_end);

	// TODO check if there are more segments at this point - if so, the length of the segment before
	// does not change!
	if (begin_it != this->t.begin()) {
		--begin_it;
		notify_begin = true;
	}

	this->t.remove(n._imap_begin);

	if (notify_begin) {
		NodeTraits::on_length_changed(*begin_it);
	}

	if (end_it != this->t.begin()) {
		--end_it;
		notify_end = true;
	}

	this->t.remove(n._imap_end);

	if (notify_end) {
		NodeTraits::on_length_changed(*end_it);
	}
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
				: lower(other.lower), upper(other.upper), t(other.t)
{}

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::IteratorBase(
				const InnerIterator & it, ITree * t_in)
				: lower(it), upper(it), t(t_in)
{
	if (this->lower == this->t->end()) {
		this->upper = this->t->end();
	} else {
		this->upper = this->lower + 1;

		if (this->lower->point == this->upper->point) {
			this->operator++();
		}
	}
}

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
ConcreteIterator &
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator=(
				const ConcreteIterator & other)
{
	this->lower = other.lower;
	this->upper = other.upper;
	this->t = other.t;
	return *this;
};

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
ConcreteIterator &
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator=(
				ConcreteIterator && other)
{
	this->lower = std::move(other.lower);
	this->upper = std::move(other.upper);
	this->t = other.t;
	return *this;
};

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
bool
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator==(
				const ConcreteIterator & other) const
{
	return other.upper == this->upper;
}

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
bool
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator!=(
				const ConcreteIterator & other) const
{
	return other.upper != this->upper;
}


template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
ConcreteIterator &
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator++()
{
	this->lower = this->upper;
	++(this->upper);

	while ((this->upper != this->t->end()) && (this->lower->point == this->upper->point)) {
		this->lower = this->upper;
		++(this->upper);
	}

	return *(static_cast<ConcreteIterator *>(this));
}

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
ConcreteIterator
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator++(int)
{
	ConcreteIterator buf(*(static_cast<ConcreteIterator *>(this)));

	this->operator++();

	return buf;
}

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
ConcreteIterator &
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator+=(size_t steps)
{
	for (unsigned int i = 0 ; i < steps ; ++i) {
		this->operator++();
	}

	return *this;
};

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
ConcreteIterator
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator+(size_t steps) const
{
	iterator it(this->lower + steps, this->t);
	return it;
};

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
ConcreteIterator &
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator--()
{

	this->upper = this->lower;
	--(this->lower);

	while ((this->lower != this->t.begin()) && (this->lower->point == this->upper->point)) {
		this->upper = this->lower;
		--(this->lower);
	}

	// If we ran into a stack of zero-length segments at the beginning, go to the first non-zero
	// length segment
	if ((this->lower->point == this->upper->point)) {
		this->operator++();
	}

	return *this;
};

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
ConcreteIterator
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator--(int)
{
	iterator buf(*this);

	this->operator--();

	return buf;
};

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
typename IntervalMap<Node, NodeTraits, Tag>::template IteratorBase<ConcreteIterator,
                                                                   InnerIterator>::reference
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator*() const
{
	return *(this->lower);
}

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
typename IntervalMap<Node, NodeTraits, Tag>::template IteratorBase<ConcreteIterator,
                                                                   InnerIterator>::pointer
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator->() const
{
	return this->lower->operator->();
}

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
typename IntervalMap<Node, NodeTraits, Tag>::key_type
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::get_lower() const
{
	return this->lower->point;
}

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
typename IntervalMap<Node, NodeTraits, Tag>::key_type
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::get_upper() const
{
	return this->upper->point;
}

template <class Node, class NodeTraits, int Tag>
template<class ConcreteIterator, class InnerIterator>
const typename IntervalMap<Node, NodeTraits, Tag>::value_type &
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::get_value() const
{
	return this->lower->aggregate;
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