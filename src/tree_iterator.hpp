#pragma once
#ifndef YGG_TREE_ITERATOR_HPP
#define YGG_TREE_ITERATOR_HPP

#include <cstddef>
#include <iterator>

namespace ygg {
namespace internal {

/**
 * @brief Iterator over elements in a tree
 *
 * This class represents an iterator over elements in a binary search tree. The
 * iterator is an input iterator in terms of STL iterators, thus it provides
 * only basic functionality.
 *
 * *Warning*: For efficiency reasons, it is currently not possible to
 * decrement the end() iterator!
 */
template <class ConcreteIterator, class Node, class NodeInterface, bool reverse>
class IteratorBase {
public:
  /// @cond INTERNAL
  typedef ptrdiff_t difference_type;
  typedef Node value_type;
  typedef Node & reference;
  typedef Node * pointer;
  typedef std::input_iterator_tag iterator_category;

  IteratorBase();
  IteratorBase(Node * n);
  IteratorBase(const ConcreteIterator & other);

  ConcreteIterator & operator=(const ConcreteIterator & other);
  ConcreteIterator & operator=(ConcreteIterator && other);

  bool operator==(const ConcreteIterator & other) const;
  bool operator!=(const ConcreteIterator & other) const;

  ConcreteIterator & operator++();
  ConcreteIterator operator++(int);
  ConcreteIterator & operator+=(size_t steps);
  ConcreteIterator operator+(size_t steps) const;

  ConcreteIterator & operator--();
  ConcreteIterator operator--(int);
  ConcreteIterator & operator-=(size_t steps);
  ConcreteIterator operator-(size_t steps) const;

  reference operator*() const;
  pointer operator->() const;

  IteratorBase<ConcreteIterator, Node, NodeInterface, !reverse>
  get_reverse() const;

protected:
  /*
   * Dispatch methods to switch the meaning of ++ / -- via SFINAE based on
   * the value of reverse
   */
  template <bool inner_reverse = reverse>
  typename std::enable_if<inner_reverse, void>::type
  dispatch_operator_pp()
  {
    this->step_back();
  }
  template <bool inner_reverse = reverse>
  typename std::enable_if<!inner_reverse, void>::type
  dispatch_operator_pp()
  {
    this->step_forward();
  }
  template <bool inner_reverse = reverse>
  typename std::enable_if<inner_reverse, void>::type
  dispatch_operator_mm()
  {
    this->step_forward();
  }
  template <bool inner_reverse = reverse>
  typename std::enable_if<!inner_reverse, void>::type
  dispatch_operator_mm()
  {
    this->step_back();
  }

  /*
   * Actual implementation of "going forwards" and "going backwards"
   */
  void step_forward();
  void step_back();

  Node * n;

  using my_type = IteratorBase<ConcreteIterator, Node, NodeInterface, reverse>;
  /// @endcond
};

} // namespace internal
} // namespace ygg

#include "tree_iterator.cpp"

#endif
