//
// Created by lukas on 04.09.17.
//

#include "intervalmap.hpp"

namespace ygg {

template <class Node, class NodeTraits, class Tag>
void
IntervalMap<Node, NodeTraits, Tag>::insert_segment(Segment *seg)
{
	key_type & point = seg->point;

	auto upper_bound_it = this->t.upper_bound(point);
	Segment * ub_repr = nullptr;
	if (upper_bound_it != this->t.end()) {
		ub_repr = &*upper_bound_it;
	}

	this->t.insert(*seg);

	NodeTraits::on_segment_inserted(*seg);

	/*
	 * Insert begin segment into the segment list
	 */
	if (ub_repr != nullptr) {
		if (ub_repr->point > point) {
			/* We are not at the same point as the upper-bounded representative; it is truly after us.
			 * This means two things:
			 *
			 * - We can take our value from the representative's predecessor (as long as it has one)
			 *    - If it has no predecessor, we are the first in the list and initialize to 0
			 * - We must insert ourselves as new representative before the upper-bounded repr
			 */
			this->l.insert(ub_repr, seg);
			NodeTraits::on_length_changed(*seg);

			auto list_it = this->l.iterator_to(*seg);
			if (list_it != this->l.begin()) {
				// Take the value of the upper-bounds old predecessor
				list_it--;
				seg->aggregate = list_it->aggregate;
				NodeTraits::on_length_changed(*seg);
			} else {
				// We are now the first segment! Initialize value to zero.
				seg->aggregate = value_type();
			}

		} else {
			/* We are at the same point as the upper-bounded repr. We…
			 *
			 * - can take the value of the upper-bounded repr.
			 * - must replace the upper-bounded repr iff we were inserted directly before it in the tree
			 */
			seg->aggregate = upper_bound_it->aggregate;

			auto seg_it = this->t.iterator_to(*seg);
			auto repr_it = this->t.iterator_to(*ub_repr);
			if (++seg_it == repr_it) {
				// we were inserted directly before the representative and have to replace it!
				this->l.insert(ub_repr, seg);
				this->l.remove(ub_repr);

				NodeTraits::on_length_changed(*ub_repr);
				NodeTraits::on_length_changed(*seg);
			}
		}
	} else {
		/* We are the very last element in the list. Insert at the end and take the aggregate from
		 * the previous last element. */
		this->l.insert(nullptr, seg);

		auto list_it = this->l.iterator_to(*seg);
		if (list_it != this->l.begin()) {
			--list_it;
			seg->aggregate = list_it->aggregate;
			NodeTraits::on_length_changed(*list_it);
		} else {
			// there is no other element in the list. Initialize to zero.
			seg->aggregate = value_type();
		}
	}
}

template <class Node, class NodeTraits, class Tag>
void
IntervalMap<Node, NodeTraits, Tag>::insert(Node &n)
{
	// TODO this is redundant.
	n._imap_begin.point = NodeTraits::get_lower(n);
	n._imap_end.point = NodeTraits::get_upper(n);


	this->insert_segment(&n._imap_begin);
	this->insert_segment(&n._imap_end);

	auto it = this->t.find(n._imap_begin.point); // guaranteed to be the first (in tree order)
																			         // segment with this point

	while (it->point < NodeTraits::get_upper(n)) {
		value_type old_val = it->aggregate;
		it->aggregate += NodeTraits::get_value(n);

		NodeTraits::on_value_changed(*it, old_val, it->aggregate);

		it++;
	}
}

template <class Node, class NodeTraits, class Tag>
void
IntervalMap<Node, NodeTraits, Tag>::remove_segment(Segment *seg)
{
	auto tree_it = this->t.iterator_to(seg);
	if ((tree_it == this->t.begin()) || ((tree_it - 1)->point != seg->point)) {
		// We are a representatitve and must delete from the list!
		auto next_it = tree_it + 1;
		if ((next_it != this->t.end()) && (next_it->point == seg->point)) {
			// The next entry replaces us!
			Segment * next_entry = nullptr;
			auto list_it = this->l.iterator_to(seg);
			++list_it;
			if (list_it != this->l.end()) {
				next_entry = *list_it;
			}

			this->l.insert(next_entry, *next_it);

			NodeTraits::on_length_changed(*next_it);
		}

		Segment * list_pred = nullptr;
		if (this->l.iterator_to(*seg) != this->l.begin()) {
			list_pred = * this->l.iterator_to(*seg) - 1;
		}

		this->l.remove(seg);

		if (list_pred != nullptr) {
			NodeTraits::on_length_changed(*list_pred);
		}
	}

	this->t.remove(seg);
}

template <class Node, class NodeTraits, class Tag>
void
IntervalMap<Node, NodeTraits, Tag>::remove(Node &n)
{
  auto it = this->t.find(n._imap_begin.point);
	typename Node::key_t stop_point = NodeTraits::get_upper(n);

	while (it->point < stop_point) {
		value_type old_val = it->aggregate;
		it->aggregate -= NodeTraits::get_value(n);
		NodeTraits::on_value_changed(*it, old_val, it->aggregate);
		it++;
	}

	this->remove_segment(& n._imap_end);
	this->remove_segment(& n._imap_begin);
}

template <class Node, class NodeTraits, class Tag>
template<class ConcreteIterator, class InnerIterator>
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::IteratorBase()
	: l(nullptr)
{}

template <class Node, class NodeTraits, class Tag>
template<class ConcreteIterator, class InnerIterator>
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::IteratorBase(
				const ConcreteIterator & other)
				: inner(other.inner), l(other.l)
{}

template <class Node, class NodeTraits, class Tag>
template<class ConcreteIterator, class InnerIterator>
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::IteratorBase(
				const InnerIterator & it, SegList * l_in)
				: inner(it), l(l_in)
{
	if ((inner == this->l->back()) && (this->l->back() != this->l->end())) {
		inner++; // The last entry is not really a segment
	}
}

template <class Node, class NodeTraits, class Tag>
template<class ConcreteIterator, class InnerIterator>
ConcreteIterator &
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator=(
				const ConcreteIterator & other)
{
	this->inner = other.inner;
	this->l = other.l;

	return *this;
};

template <class Node, class NodeTraits, class Tag>
template<class ConcreteIterator, class InnerIterator>
ConcreteIterator &
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator=(
				ConcreteIterator && other)
{
	this->inner = std::move(other.inner);
	this->l = other.l;

	return *this;
};

template <class Node, class NodeTraits, class Tag>
template<class ConcreteIterator, class InnerIterator>
bool
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator==(
				const ConcreteIterator & other) const
{
	return (other.inner == this->inner) && (other.l == this->l);
}

template <class Node, class NodeTraits, class Tag>
template<class ConcreteIterator, class InnerIterator>
bool
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator!=(
				const ConcreteIterator & other) const
{
	return (other.inner != this->inner) || (other.l != this->l);
}


template <class Node, class NodeTraits, class Tag>
template<class ConcreteIterator, class InnerIterator>
ConcreteIterator &
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator++()
{
	this->inner++;
	if (this->inner == this->l->back()) {
		this->inner++; // last entry is not really a segment
	}

	return *(static_cast<ConcreteIterator *>(this));
}

template <class Node, class NodeTraits, class Tag>
template<class ConcreteIterator, class InnerIterator>
ConcreteIterator
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator++(int)
{
	ConcreteIterator buf(*(static_cast<ConcreteIterator *>(this)));

	this->operator++();

	return buf;
}

template <class Node, class NodeTraits, class Tag>
template<class ConcreteIterator, class InnerIterator>
ConcreteIterator &
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator+=(size_t steps)
{
	for (unsigned int i = 0 ; i < steps ; ++i) {
		this->operator++();
	}

	return *this;
};

template <class Node, class NodeTraits, class Tag>
template<class ConcreteIterator, class InnerIterator>
ConcreteIterator
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator+(size_t steps) const
{
	ConcreteIterator it(this->inner + steps, this->l);
	return it;
};

template <class Node, class NodeTraits, class Tag>
template<class ConcreteIterator, class InnerIterator>
ConcreteIterator &
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator--()
{
	if (this->inner == this->l.end()) {
		this->inner = this->l.back() - 1;
	} else {
		this->inner--;
	}

	return *this;
};

template <class Node, class NodeTraits, class Tag>
template<class ConcreteIterator, class InnerIterator>
ConcreteIterator
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator--(int)
{
	ConcreteIterator buf(*this);

	this->operator--();

	return buf;
};

template <class Node, class NodeTraits, class Tag>
template<class ConcreteIterator, class InnerIterator>
typename IntervalMap<Node, NodeTraits, Tag>::template IteratorBase<ConcreteIterator,
                                                                   InnerIterator>::reference
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator*() const
{
	return *(this->inner);
}

template <class Node, class NodeTraits, class Tag>
template<class ConcreteIterator, class InnerIterator>
typename IntervalMap<Node, NodeTraits, Tag>::template IteratorBase<ConcreteIterator,
                                                                   InnerIterator>::pointer
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator->() const
{
	return &*this->inner;
}

template <class Node, class NodeTraits, class Tag>
template<class ConcreteIterator, class InnerIterator>
typename IntervalMap<Node, NodeTraits, Tag>::key_type
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::get_lower() const
{
	return this->inner->point;
}

template <class Node, class NodeTraits, class Tag>
template<class ConcreteIterator, class InnerIterator>
typename IntervalMap<Node, NodeTraits, Tag>::key_type
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::get_upper() const
{
	return (this->inner + 1)->point;
}

template <class Node, class NodeTraits, class Tag>
template<class ConcreteIterator, class InnerIterator>
const typename IntervalMap<Node, NodeTraits, Tag>::value_type &
IntervalMap<Node, NodeTraits, Tag>::IteratorBase<ConcreteIterator, InnerIterator>::get_value() const
{
	return this->inner->aggregate;
}

template <class Node, class NodeTraits, class Tag>
typename IntervalMap<Node, NodeTraits, Tag>::const_iterator
IntervalMap<Node, NodeTraits, Tag>::begin() const
{
	return const_iterator(this->l.begin(), & this->l);
};

template <class Node, class NodeTraits, class Tag>
typename IntervalMap<Node, NodeTraits, Tag>::const_iterator
IntervalMap<Node, NodeTraits, Tag>::end() const
{
	return const_iterator(this->l.end(), & this->l);
};

template <class Node, class NodeTraits, class Tag>
typename IntervalMap<Node, NodeTraits, Tag>::iterator
IntervalMap<Node, NodeTraits, Tag>::begin()
{
	return iterator(this->l.begin(), & this->l);
};

template <class Node, class NodeTraits, class Tag>
typename IntervalMap<Node, NodeTraits, Tag>::iterator
IntervalMap<Node, NodeTraits, Tag>::end()
{
	return iterator(this->l.end(), & this->l);
};

template <class Node, class NodeTraits, class Tag>
typename IntervalMap<Node, NodeTraits, Tag>::iterator
IntervalMap<Node, NodeTraits, Tag>::find_upper_bound_representative(typename Node::key_type point)
{
	typename SegList::iterator inner_iterator = this->l.iterator_to(*this->t.upper_bound(point));
	return iterator(inner_iterator, & this->l);
}

template <class Node, class NodeTraits, class Tag>
void
IntervalMap<Node, NodeTraits, Tag>::dbg_verify()
{
	this->dbg_verify_list();
}

template <class Node, class NodeTraits, class Tag>
void
IntervalMap<Node, NodeTraits, Tag>::dbg_verify_list()
{
	auto list_it = this->l.begin();
	auto tree_it = this->t.begin();

	while (list_it != this->l.end()) {
		assert(&*list_it == &*tree_it);

		// Advance list once, tree until the next point is found
		key_type old_point = tree_it->point;
		while ((tree_it != this->t.end()) && (tree_it->point == old_point)) {
			++tree_it;
		}
		++list_it;
	}

	assert(tree_it == this->t.end());
}

} // namespace ygg