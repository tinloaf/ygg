#ifndef INTERVALTREE_HPP
#define INTERVALTREE_HPP

#include <algorithm>
#include <string>
#include <iostream>

#include "rbtree.hpp"

namespace ygg {
  namespace utilities {
    template<class Node, class NodeTraits, bool skipfirst, class Comparable>
    Node * find_next_overlapping(Node * cur, const Comparable & q);

    template<class KeyType>
    class DummyRange : public std::pair<KeyType, KeyType> {
    public:
      DummyRange(KeyType lower, KeyType upper);
    };

	  template<class Node, class NodeTraits>
	  class IntervalCompare {
	  public:
		  template<class T1, class T2>
		  bool operator()(const T1 & lhs, const T2 & rhs) const;
	  };

	  // TODO add a possibility for bulk updates
	  template<class Node, class NodeTraits>
	  class ExtendedNodeTraits : public NodeTraits {
	  public:
		  // TODO these can probably made more efficient
		  static void leaf_inserted(Node & node);
		  static void fix_node(Node & node);
		  static void rotated_left(Node & node);
		  static void rotated_right(Node & node);
		  static void deleted_below(Node & node);
		  static void swapped(Node & n1, Node & n2);

		  // Make our DummyRange comparable
		  static typename NodeTraits::key_type get_lower(const utilities::DummyRange<typename NodeTraits::key_type> & range);
		  static typename NodeTraits::key_type get_upper(const utilities::DummyRange<typename NodeTraits::key_type> & range);
	  };
  } // namespace utilities

template<class Node, class NodeTraits, class Options = TreeOptions<TreeFlags::MULTIPLE>,
				int Tag = 0>
class ITreeNodeBase : public RBTreeNodeBase<Node, Options, Tag> {
public:
  typename NodeTraits::key_type    _it_max_upper;
};


template<class Node, class NodeTraits, class Options = TreeOptions<TreeFlags::MULTIPLE>,
				int Tag = 0>
class IntervalTree : public RBTree<Node, utilities::ExtendedNodeTraits<Node, NodeTraits>,
                                   Options, Tag, utilities::IntervalCompare<Node, NodeTraits>>
{
public:
  using Key = typename NodeTraits::key_type;
  // TODO why do I need to specify this again?
  using EqualityList = utilities::EqualityListHelper<Node, Node::_rbt_multiple,
                                                     utilities::IntervalCompare<Node, NodeTraits>>;
  using ENodeTraits = utilities::ExtendedNodeTraits<Node, NodeTraits>;
  using BaseTree = RBTree<Node, utilities::ExtendedNodeTraits<Node, NodeTraits>, Options, Tag,
					                utilities::IntervalCompare<Node, NodeTraits>>;

	using INB = ITreeNodeBase<Node, NodeTraits, Options, Tag>;
	static_assert(std::is_base_of<INB, Node>::value, "Node class not properly derived from "
					"ITreeNodeBase");

  IntervalTree();

  bool verify_integrity() const;
  void dump_to_dot(const std::string & filename) const;

  // Iteration of sets of intervals
  template <class Comparable>
  class QueryResult {
  public:
    class const_iterator {
    public:
      typedef ptrdiff_t                         difference_type;
      typedef Node                              value_type;
      typedef const Node &                      const_reference;
      typedef const Node *                      const_pointer;
      typedef std::input_iterator_tag           iterator_category;

      const_iterator (Node * n, const Comparable & q);
      const_iterator (const const_iterator & other);
      ~const_iterator();

      const_iterator& operator=(const const_iterator & other);

      bool operator==(const const_iterator & other) const;
      bool operator!=(const const_iterator & other) const;

      const_iterator& operator++();
      const_iterator operator++(int);

      const_reference operator*() const;
      const_pointer operator->() const;

    private:
      Node * n;
      Comparable q;
    };

    QueryResult(Node * n, const Comparable & q);

    const_iterator begin() const;
    const_iterator end() const;
  private:
    Node * n;
    Comparable q;
  };

  template<class Comparable>
  QueryResult<Comparable> query(const Comparable & q) const;

  template<class Comparable>
  typename BaseTree::template const_iterator<false> interval_upper_bound(const Comparable & query_range) const;

  // TODO FIXME this is actually very specific?
  void fixup_maxima(Node & lowest);

private:
  bool verify_maxima(Node * n) const;
};

#include "intervaltree.cpp"

} // namespace ygg

#endif // INTERVALTREE_HPP
