//
// Created by lukas on 04.09.17.
//

#include "intervalmap.hpp"

namespace ygg {

template <class Node, class NodeTraits, class Options, class Tag>
void
IntervalMap<Node, NodeTraits, Options, Tag>::repr_now_equal(Segment * a,
                                                            Segment * b)
{
  // std::cout << "  ---> Now equal: " << a << " / " << b << "\n";
  Segment * new_repr = nullptr;
  if (a != nullptr) {
    new_repr = a->repr;
  } else {
    // There is no previous segment / representative! Promote the next segment
    // to a representative.
    auto head_it = this->l.iterator_to(*b);
    if (head_it != this->l.back()) {
      new_repr = &*(head_it + 1);

      if (new_repr->repr == b) {
	// actually promote it!
	Segment * successor = nullptr;
	if (this->repr_list.iterator_to(*b) != this->repr_list.back()) {
	  successor = &*(this->repr_list.iterator_to(*b) + 1);
	}
	this->repr_list.insert(successor, new_repr);
	new_repr->repr = new_repr;
      }
    } // else: we were the last element. Nothing to be done.
  }

  if (new_repr != nullptr) {
    auto it = this->l.iterator_to(*new_repr);
    while ((it != this->l.end()) && (it->aggregate == new_repr->aggregate)) {
      // update representatives of all segments that pointed to b
      it->repr = new_repr;
      ++it;
    }
  }

  this->repr_list.remove(b);
}

template <class Node, class NodeTraits, class Options, class Tag>
void
IntervalMap<Node, NodeTraits, Options, Tag>::repr_now_different(Segment * a,
                                                                Segment * b)
{
  // std::cout << "  ---> Now different: " << a << " / " << b << "\n";

  Segment * next = nullptr;
  if (a != nullptr) {
    if (b != nullptr) {
      auto repr_it = this->repr_list.iterator_to(*a->repr);
      ++repr_it;
      if (repr_it != this->repr_list.end()) {
	next = &*repr_it;
      }
    } else {
      // we inserted at the very end! We need to insert a at the end. Switch a
      // and b
      b = a;
      next = nullptr;
    }
  } else {
    // insert at the begin
    if (this->repr_list.begin() != this->repr_list.end()) {
      next = &*this->repr_list.begin();
    }
  }
  this->repr_list.insert(next, b);

  auto it = this->l.iterator_to(*b);
  while ((it != this->l.end()) && (it->aggregate == b->aggregate)) {
    it->repr = b;
    ++it;
  }
}

template <class Node, class NodeTraits, class Options, class Tag>
void
IntervalMap<Node, NodeTraits, Options, Tag>::repr_replaced(
    Segment * original, Segment * replacement)
{
  // We only need to do anything if original is a representative!
  if (original->repr != original) {
    return;
  }

  // std::cout << "  ---> Replaced: " << original << " by " << replacement <<
  // "\n";

  Segment * next = nullptr;
  auto repr_it = this->repr_list.iterator_to(*original);
  ++repr_it;
  if (repr_it != this->repr_list.end()) {
    next = &*repr_it;
  }
  this->repr_list.insert(next, replacement);
  this->repr_list.remove(original);

  replacement->repr = replacement;

  auto it = this->l.iterator_to(*replacement);
  ++it;
  while ((it != this->l.end()) && (it->repr == original)) {
    it->repr = replacement;
    ++it;
  }
}

template <class Node, class NodeTraits, class Options, class Tag>
typename IntervalMap<Node, NodeTraits, Options, Tag>::Segment *
IntervalMap<Node, NodeTraits, Options, Tag>::insert_segment(Segment * seg)
{
  // std::cout << "   Inserting " << seg << "\n";
  key_type & point = seg->point;

  auto lower_bound_it = this->t.lower_bound(point);
  Segment * lb_head = nullptr;
  if (lower_bound_it != this->t.end()) {
    lb_head = &*lower_bound_it;
  }
  auto head_it = this->t.iterator_to(*lb_head);

  // std::cout << "     Uppper-Bounded Head: " << &*lb_head << "\n";

  this->t.insert(*seg);

  NodeTraits::on_segment_inserted(*seg);

  /*
   * Insert begin segment into the segment list
   */
  if (lb_head != nullptr) {
    if (lb_head->point > point) {
      // std::cout << "     Not at the same point!\n";
      /* We are not at the same point as the upper-bounded header; it is truly
       * after us. This means two things:
       *
       * - We can take our value from the header's predecessor (as long as it
       * has one)
       *    - If it has no predecessor, we are the first in the list and
       * initialize to 0
       * - We must insert ourselves as new header before the upper-bounded head
       */
      this->l.insert(lb_head, seg);
      NodeTraits::on_length_changed(*seg);

      auto list_it = this->l.iterator_to(*seg);
      if (list_it != this->l.begin()) {
	// Take the value of the upper-bounds old predecessor
	list_it--;
	seg->aggregate = list_it->aggregate;
	seg->repr = list_it->repr;
	NodeTraits::on_length_changed(*seg);
      } else {
	// We are now the first segment! Initialize value to zero.
	seg->aggregate = value_type();
	seg->repr = nullptr;
      }

      return seg;

    } else {
      // std::cout << "     At the same point!\n";
      /* We are at the same point as the upper-bounded head. We…
       *
       * - can take the value of the upper-bounded head.
       * - must replace the upper-bounded head iff we were inserted directly
       * before it in the tree
       */
      seg->aggregate = lower_bound_it->aggregate;

      auto seg_it = this->t.iterator_to(*seg);
      if (++seg_it == head_it) {
	// std::cout << "     Replacing the head!\n";
	// we were inserted directly before the header and have to replace it!
	this->l.insert(lb_head, seg);
	this->l.remove(lb_head);

	// TODO FIXME these should be called with representatives!
	NodeTraits::on_length_changed(*lb_head);
	NodeTraits::on_length_changed(*seg);

	seg->repr = lb_head->repr;
	this->repr_replaced(lb_head, seg);

	return seg;
      } else {
	// the upper-bounded header is our header!
	seg->repr = lb_head->repr;
	return lb_head;
      }
    }
  } else {
    // std::cout << "     At the end!\n";
    /* We are the very last element in the list. Insert at the end and take the
     * aggregate from the previous last element. */
    this->l.insert(nullptr, seg);

    auto list_it = this->l.iterator_to(*seg);
    if (list_it != this->l.begin()) {
      --list_it;
      seg->aggregate = list_it->aggregate;
      seg->repr = list_it->repr;
      NodeTraits::on_length_changed(*list_it);
    } else {
      // there is no other element in the list. Initialize to zero.
      seg->aggregate = value_type();
      seg->repr = nullptr;
    }

    return seg;
  }
}

template <class Node, class NodeTraits, class Options, class Tag>
void
IntervalMap<Node, NodeTraits, Options, Tag>::insert(Node & n)
{
  this->s.add(1);

  // TODO this is redundant.
  // std::cout << " ====> Insert. Begin Segment: " << &n.NB::_imap_begin << "
  // End Segment: "
  //          << &n.NB::_imap_end << "\n";
  n.NB::_imap_begin.point = NodeTraits::get_lower(n);
  n.NB::_imap_end.point = NodeTraits::get_upper(n);

  Segment * begin_head = this->insert_segment(&n.NB::_imap_begin);
  Segment * end_head = this->insert_segment(&n.NB::_imap_end);

  bool begin_was_equal = begin_head->repr != begin_head;
  bool end_was_equal = end_head->repr != end_head;

  auto it = this->t.iterator_to(*begin_head);

  while (it->point < NodeTraits::get_upper(n)) {
    value_type old_val = it->aggregate;
    it->aggregate += NodeTraits::get_value(n);

    NodeTraits::on_value_changed(*it, old_val, it->aggregate);

    ++it;
  }

  auto begin_it = this->l.iterator_to(*begin_head);
  bool begin_now_equal = false;
  if ((begin_it != this->l.begin()) &&
      ((begin_it - 1)->aggregate == begin_it->aggregate)) {
    begin_now_equal = true;
  }

  auto end_it = this->l.iterator_to(*end_head);
  bool end_now_equal = false;
  if ((end_it - 1)->aggregate == end_it->aggregate) {
    end_now_equal = true;
  }

  // If we inserted at the very end, we're also 'not equal'…
  if (end_head->repr == nullptr) {
    end_now_equal = false;
  }

  if (begin_was_equal && !begin_now_equal) {
    Segment * predecessor = nullptr;
    if (begin_it != this->l.begin()) {
      predecessor = &*(begin_it - 1);
    }
    this->repr_now_different(predecessor, begin_head);
  } else if (!begin_was_equal && begin_now_equal) {
    Segment * predecessor = &*(begin_it - 1);
    this->repr_now_equal(predecessor, begin_head);
  }

  if (end_was_equal && !end_now_equal) {
    Segment * predecessor = &*(end_it - 1);
    this->repr_now_different(predecessor, end_head);
  } else if (!end_was_equal && end_now_equal) {
    Segment * predecessor = &*(end_it - 1);
    this->repr_now_equal(predecessor, end_head);
  }
}

template <class Node, class NodeTraits, class Options, class Tag>
void
IntervalMap<Node, NodeTraits, Options, Tag>::remove_segment(Segment * seg)
{
  auto tree_it = this->t.iterator_to(*seg);
  if ((tree_it == this->t.begin()) || ((tree_it - 1)->point != seg->point)) {
    // We are a head and must delete from the list!
    auto next_it = tree_it + 1;
    if ((next_it != this->t.end()) && (next_it->point == seg->point)) {
      // The next entry replaces us!
      Segment * next_entry = nullptr;
      auto list_it = this->l.iterator_to(*seg);
      ++list_it;
      if (list_it != this->l.end()) {
	next_entry = &*list_it;
      }

      this->l.insert(next_entry, &*next_it);
      next_entry->repr = seg->repr;

      if (seg->repr == seg) {
	// We are also a representative and are being replaced by next_entry!
	this->repr_replaced(seg, next_entry);
      }

      NodeTraits::on_length_changed(*next_it);
    }

    Segment * list_pred = nullptr;
    if (this->l.iterator_to(*seg) != this->l.begin()) {
      list_pred = &*(this->l.iterator_to(*seg) - 1);
    }

    this->l.remove(seg);

    if (list_pred != nullptr) {
      NodeTraits::on_length_changed(*list_pred);
    }
  }

  this->t.remove(*seg);
}

template <class Node, class NodeTraits, class Options, class Tag>
size_t
IntervalMap<Node, NodeTraits, Options, Tag>::size() const
{
  return this->s.get();
}

template <class Node, class NodeTraits, class Options, class Tag>
bool
IntervalMap<Node, NodeTraits, Options, Tag>::empty() const
{
  return this->t.empty();
}

template <class Node, class NodeTraits, class Options, class Tag>
void
IntervalMap<Node, NodeTraits, Options, Tag>::remove(Node & n)
{
  this->s.reduce(1);

  auto it = this->t.find(n.NB::_imap_begin.point);
  key_type stop_point = NodeTraits::get_upper(n);

  Segment * begin_head = this->get_head(&n.NB::_imap_begin);
  Segment * end_head = this->get_head(&n.NB::_imap_end);

  bool begin_was_equal = begin_head->repr != begin_head;
  bool end_was_equal = end_head->repr != end_head;

  while (it->point < stop_point) {
    value_type old_val = it->aggregate;
    it->aggregate -= NodeTraits::get_value(n);
    NodeTraits::on_value_changed(*it, old_val, it->aggregate);
    it++;
  }

  bool begin_now_equal = true;
  auto begin_it = this->l.iterator_to(*begin_head);
  if ((begin_it != this->l.begin()) &&
      ((begin_it - 1)->aggregate != begin_it->aggregate)) {
    begin_now_equal = false;
  }

  if (begin_was_equal && !begin_now_equal) {
    this->repr_now_different(&*(begin_it - 1), begin_head);
  } else if (!begin_was_equal && begin_now_equal) {
    if (begin_it != this->l.begin()) {
      this->repr_now_equal(&*(begin_it - 1), begin_head);
    } else {
      this->repr_now_equal(nullptr, begin_head);
    }
  }

  bool end_now_equal = true;
  auto end_it = this->l.iterator_to(*end_head);
  if ((end_it - 1)->aggregate != end_it->aggregate) {
    /*
     * We have to check if a repr_now_equal on the begin has already promoted
     * the end to a representative…
     */
    if (end_it->repr == &*end_it) {
      // end_it was already promoted to a representative!
    } else {
      end_now_equal = false;
    }
  }

  if (end_was_equal && !end_now_equal) {
    this->repr_now_different(&*(end_it - 1), end_head);
  } else if (!end_was_equal && end_now_equal) {
    this->repr_now_equal(&*(end_it - 1), end_head);
  }

  this->remove_segment(&n.NB::_imap_end);
  this->remove_segment(&n.NB::_imap_begin);
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class ConcreteIterator, class InnerIterator>
IntervalMap<Node, NodeTraits, Options,
            Tag>::IteratorBase<ConcreteIterator, InnerIterator>::IteratorBase()
    : l(nullptr)
{}

template <class Node, class NodeTraits, class Options, class Tag>
template <class ConcreteIterator, class InnerIterator>
IntervalMap<Node, NodeTraits, Options, Tag>::IteratorBase<
    ConcreteIterator, InnerIterator>::IteratorBase(const ConcreteIterator &
                                                       other)
    : inner(other.inner), l(other.l)
{}

template <class Node, class NodeTraits, class Options, class Tag>
template <class ConcreteIterator, class InnerIterator>
IntervalMap<Node, NodeTraits, Options, Tag>::IteratorBase<
    ConcreteIterator, InnerIterator>::IteratorBase(const InnerIterator & it,
                                                   RepresentativeSegList * l_in)
    : inner(it), l(l_in)
{
  if ((inner == this->l->back()) && (this->l->back() != this->l->end())) {
    inner++; // The last entry is not really a segment
  }
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class ConcreteIterator, class InnerIterator>
ConcreteIterator &
IntervalMap<Node, NodeTraits, Options, Tag>::IteratorBase<ConcreteIterator,
                                                          InnerIterator>::
operator=(const ConcreteIterator & other)
{
  this->inner = other.inner;
  this->l = other.l;

  return *this;
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class ConcreteIterator, class InnerIterator>
ConcreteIterator &
IntervalMap<Node, NodeTraits, Options, Tag>::IteratorBase<ConcreteIterator,
                                                          InnerIterator>::
operator=(ConcreteIterator && other)
{
  this->inner = std::move(other.inner);
  this->l = other.l;

  return *this;
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class ConcreteIterator, class InnerIterator>
bool
IntervalMap<Node, NodeTraits, Options, Tag>::IteratorBase<ConcreteIterator,
                                                          InnerIterator>::
operator==(const ConcreteIterator & other) const
{
  return (other.inner == this->inner) && (other.l == this->l);
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class ConcreteIterator, class InnerIterator>
bool
IntervalMap<Node, NodeTraits, Options, Tag>::IteratorBase<ConcreteIterator,
                                                          InnerIterator>::
operator!=(const ConcreteIterator & other) const
{
  return (other.inner != this->inner) || (other.l != this->l);
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class ConcreteIterator, class InnerIterator>
ConcreteIterator &
IntervalMap<Node, NodeTraits, Options,
            Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator++()
{
  this->inner++;
  if (this->inner == this->l->back()) {
    this->inner++; // last entry is not really a segment
  }

  return *(static_cast<ConcreteIterator *>(this));
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class ConcreteIterator, class InnerIterator>
ConcreteIterator
IntervalMap<Node, NodeTraits, Options,
            Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator++(int)
{
  ConcreteIterator buf(*(static_cast<ConcreteIterator *>(this)));

  this->operator++();

  return buf;
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class ConcreteIterator, class InnerIterator>
ConcreteIterator &
IntervalMap<Node, NodeTraits, Options, Tag>::IteratorBase<ConcreteIterator,
                                                          InnerIterator>::
operator+=(size_t steps)
{
  for (unsigned int i = 0; i < steps; ++i) {
    this->operator++();
  }

  return *this;
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class ConcreteIterator, class InnerIterator>
ConcreteIterator
IntervalMap<Node, NodeTraits, Options, Tag>::IteratorBase<ConcreteIterator,
                                                          InnerIterator>::
operator+(size_t steps) const
{
  ConcreteIterator it(this->inner + steps, this->l);
  return it;
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class ConcreteIterator, class InnerIterator>
ConcreteIterator &
IntervalMap<Node, NodeTraits, Options,
            Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator--()
{
  if (this->inner == this->l.end()) {
    this->inner = this->l.back() - 1;
  } else {
    this->inner--;
  }

  return *this;
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class ConcreteIterator, class InnerIterator>
ConcreteIterator
IntervalMap<Node, NodeTraits, Options,
            Tag>::IteratorBase<ConcreteIterator, InnerIterator>::operator--(int)
{
  ConcreteIterator buf(*this);

  this->operator--();

  return buf;
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class ConcreteIterator, class InnerIterator>
typename IntervalMap<Node, NodeTraits, Options, Tag>::template IteratorBase<
    ConcreteIterator, InnerIterator>::reference
    IntervalMap<Node, NodeTraits, Options, Tag>::IteratorBase<ConcreteIterator,
                                                              InnerIterator>::
    operator*() const
{
  return *(this->inner);
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class ConcreteIterator, class InnerIterator>
typename IntervalMap<Node, NodeTraits, Options, Tag>::template IteratorBase<
    ConcreteIterator, InnerIterator>::pointer
    IntervalMap<Node, NodeTraits, Options, Tag>::IteratorBase<ConcreteIterator,
                                                              InnerIterator>::
    operator->() const
{
  return &*this->inner;
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class ConcreteIterator, class InnerIterator>
typename IntervalMap<Node, NodeTraits, Options, Tag>::key_type
IntervalMap<Node, NodeTraits, Options, Tag>::IteratorBase<
    ConcreteIterator, InnerIterator>::get_lower() const
{
  return this->inner->point;
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class ConcreteIterator, class InnerIterator>
typename IntervalMap<Node, NodeTraits, Options, Tag>::key_type
IntervalMap<Node, NodeTraits, Options, Tag>::IteratorBase<
    ConcreteIterator, InnerIterator>::get_upper() const
{
  return (this->inner + 1)->point;
}

template <class Node, class NodeTraits, class Options, class Tag>
template <class ConcreteIterator, class InnerIterator>
const typename IntervalMap<Node, NodeTraits, Options, Tag>::value_type &
IntervalMap<Node, NodeTraits, Options, Tag>::IteratorBase<
    ConcreteIterator, InnerIterator>::get_value() const
{
  return this->inner->aggregate;
}

template <class Node, class NodeTraits, class Options, class Tag>
typename IntervalMap<Node, NodeTraits, Options, Tag>::const_iterator
IntervalMap<Node, NodeTraits, Options, Tag>::begin() const
{
  return const_iterator(this->repr_list.begin(), &this->repr_list);
}

template <class Node, class NodeTraits, class Options, class Tag>
typename IntervalMap<Node, NodeTraits, Options, Tag>::const_iterator
IntervalMap<Node, NodeTraits, Options, Tag>::end() const
{
  return const_iterator(this->repr_list.end(), &this->repr_list);
}

template <class Node, class NodeTraits, class Options, class Tag>
typename IntervalMap<Node, NodeTraits, Options, Tag>::iterator
IntervalMap<Node, NodeTraits, Options, Tag>::begin()
{
  return iterator(this->repr_list.begin(), &this->repr_list);
}

template <class Node, class NodeTraits, class Options, class Tag>
typename IntervalMap<Node, NodeTraits, Options, Tag>::iterator
IntervalMap<Node, NodeTraits, Options, Tag>::end()
{
  return iterator(this->repr_list.end(), &this->repr_list);
}

template <class Node, class NodeTraits, class Options, class Tag>
typename IntervalMap<Node, NodeTraits, Options, Tag>::iterator
IntervalMap<Node, NodeTraits, Options, Tag>::find_lower_bound_representative(
    typename Node::key_type point)
{
  typename SegList::iterator inner_iterator =
      this->l.iterator_to(*this->t.lower_bound(point));
  return iterator(inner_iterator, &this->l);
}

template <class Node, class NodeTraits, class Options, class Tag>
typename IntervalMap<Node, NodeTraits, Options, Tag>::Segment *
IntervalMap<Node, NodeTraits, Options, Tag>::get_head(Segment * seg)
{
  /*
  if (seg->repr == seg) {
          return seg;
  }
  */

  typename ITree::template iterator<false> it = this->t.iterator_to(*seg);
  Segment * head;
  while (it->point == seg->point) {
    head = &*it;
    if (it == this->t.begin()) {
      break;
    }
    --it;
  }

  return head;
}

template <class Node, class NodeTraits, class Options, class Tag>
void
IntervalMap<Node, NodeTraits, Options, Tag>::dbg_verify()
{
  this->dbg_verify_list();
  this->dbg_verify_representatives();
}

template <class Node, class NodeTraits, class Options, class Tag>
void
IntervalMap<Node, NodeTraits, Options, Tag>::dbg_verify_representatives()
{
  auto head_it = this->l.begin();
  auto repr_it = this->repr_list.begin();

  /*
  //std::cout << "=================================================\n";
  for (auto & seg : this->t) {
          //std::cout << " TreeNode: " << &seg << " @" << seg.point << " : " <<
  seg.aggregate << "  (repr: "
                                          "" << seg.repr <<  ")\n";
  }
  //std::cout << "---------------------------------\n";
  for (auto & seg : this->l) {
          //std::cout << " Head: " << &seg << "  @" << seg.point << " : " <<
  seg.aggregate << "  (repr: "
                                          "" << seg.repr <<  ")\n";
  }
  //std::cout << "---------------------------------\n";

  for (auto & seg : this->repr_list) {
          //std::cout << " Repr: " << &seg << "  @" << seg.point << " : " <<
  seg.aggregate << "  (repr: "
                                          "" << seg.repr <<  ")\n";
  }
  //std::cout << "=================================================\n";
  */

  while (head_it != this->l.end()) {
    ////std::cout << "Head: " << &*head_it << "  /  Repr: " << &*repr_it <<
    ///"\n";
    assert(&*head_it == &*repr_it);
    auto val = head_it->aggregate;
    while ((head_it != this->l.end()) && (head_it->aggregate == val)) {
      assert(head_it->repr == &*repr_it);
      ////std::cout << "  Skipping Head: " << &*head_it << "\n";
      ++head_it;
    }
    ++repr_it;
  }

  assert(repr_it == this->repr_list.end());
}

template <class Node, class NodeTraits, class Options, class Tag>
void
IntervalMap<Node, NodeTraits, Options, Tag>::dbg_verify_list()
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