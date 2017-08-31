#ifndef INTERVALTREE_HPP
#define INTERVALTREE_HPP

#include <algorithm>
#include <string>
#include <iostream>

#include "rbtree.hpp"

namespace ygg {
  namespace utilities {
    template<class Node, class INB, class NodeTraits, bool skipfirst, class Comparable>
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
	  template<class Node, class NB, class NodeTraits>
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

/**
 * @brief Abstract base class for the Node Traits that need to be implemented
 *
 * Every Interval Tree needs to be supplied with a node traits class that must be derived from
 * this class. In your derived class, you must define the key_type as the type of your interval's
 * bounds, and you must implement get_lower() and get_upper() to return the interval bounds of
 * your nodes.
 */
template<class Node>
class ITreeNodeTraits {
public:
	/**
	 * @brief The type of your interval bounds. This is the type that get_lower() and get_upper()
	 * must return. This type must be comparable, i.e., operator< etc. must be implemented.
	 */
	using key_type = void;

	/**
	 * Must be implemented to return the lower bound of the interval represented by n.
	 *
	 * @param n The node whose lower interval bound should be returned.
	 * @return Must return the lower interval bound of n
	 */
	key_type get_lower(const Node & n) = delete;

	/**
	 * Must be implemented to return the upper bound of the interval represented by n.
	 *
	 * @param n The node whose upper interval bound should be returned.
	 * @return Must return the upper interval bound of n
	 */
	key_type get_upper(const Node & n) = delete;
};

/**
 * @brief Stores an Interval Tree
 *
 * This class stores an interval tree on the nodes it contains. It is implemented via the
 * 'augmented red-black tree' described by Cormen et al.
 *
 * @tparam Node 				The node class for this Interval Tree. Must be derived from ITreeNodeBase.
 * @tparam NodeTraits 	The node traits for this Interval Tree. Must be derived from
 * @tparam Options			Passed through to RBTree. See there for documentation.
 * @tparam Tag					Used to add nodes to multiple interval trees. See RBTree documentation
 * for details.
 */
template<class Node, class NodeTraits, class Options = TreeOptions<TreeFlags::MULTIPLE>,
				int Tag = 0>
class IntervalTree : public RBTree<Node,
                                   utilities::ExtendedNodeTraits<Node,
                                                                 ITreeNodeBase<Node, NodeTraits, Options, Tag>,
                                                                 NodeTraits>,
                                   Options, Tag, utilities::IntervalCompare<Node, NodeTraits>>
{
public:
  using Key = typename NodeTraits::key_type;
  // TODO why do I need to specify this again?

	using INB = ITreeNodeBase<Node, NodeTraits, Options, Tag>;
	static_assert(std::is_base_of<INB, Node>::value,
	              "Node class not properly derived from ITreeNodeBase!");

	static_assert(std::is_base_of<ITreeNodeTraits<Node>, NodeTraits>::value,
	              "NodeTraits not properly derived from ITreeNodeTraits!");

	using EqualityList = utilities::EqualityListHelper<Node, INB, Node::_rbt_multiple,
                                                     utilities::IntervalCompare<Node, NodeTraits>>;
  using ENodeTraits = utilities::ExtendedNodeTraits<Node, INB, NodeTraits>;
  using BaseTree = RBTree<Node, utilities::ExtendedNodeTraits<Node, INB, NodeTraits>, Options, Tag,
					                utilities::IntervalCompare<Node, NodeTraits>>;



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
