#ifndef RBTREE_HPP
#define RBTREE_HPP

#include <cstddef>
#include <set>
#include <cassert>

#include "options.hpp"

// Only for debugging purposes
#include <fstream>
#include <vector>

namespace ygg {
  namespace utilities {
    template<class Node, class const_iterator>
    class IteratorHelperUnspecialized {
    public:
      static const_iterator & step_forward(const_iterator & it);
      static const_iterator & step_back(const_iterator & it);
    };

    template<class Node, class const_iterator, bool reverse>
    class IteratorHelper {
    public:
      static const_iterator & operator_plusplus(const_iterator & it);
      static const_iterator & operator_minusminus(const_iterator & it);
    };

    template<class Node, class const_iterator>
    class IteratorHelper<Node, const_iterator, true>  {
    public:
      static const_iterator & operator_plusplus(const_iterator & it);
      static const_iterator & operator_minusminus(const_iterator & it);
    };

    template<class Node, class const_iterator>
    class IteratorHelper<Node, const_iterator, false>  {
    public:
      static const_iterator & operator_plusplus(const_iterator & it);
      static const_iterator & operator_minusminus(const_iterator & it);
    };

    template<class Node, bool multiple, class Compare>
    class EqualityListHelper {};

    template<class Node, class Compare>
    class EqualityListHelper<Node, true, Compare> {
    public:
      static void equality_list_insert_after(Node & node, Node * predecessor);
      static void equality_list_insert_before(Node & node, Node * successor);
      static void equality_list_delete_node(Node & node);
      static Node * equality_list_find_first(Node * node);
      static Node * equality_list_next(Node * node);
      static Node * equality_list_prev(Node * node);
      static void equality_list_swap_if_necessary(Node & n1, Node & n2);
      static bool verify(const Node & n);
    };
    template<class Node, class Compare>
    class EqualityListHelper<Node, false, Compare> {
    public:
      static void equality_list_insert_after(Node & node, Node * predecessor);
      static void equality_list_insert_before(Node & node, Node * successor);
      static void equality_list_delete_node(Node & node);
      static Node * equality_list_find_first(Node * node);
      static Node * equality_list_next(Node * node);
      static Node * equality_list_prev(Node * node);
      static void equality_list_swap_if_necessary(Node & n1, Node & n2);
      static bool verify(const Node & n);
    };

	  /// @cond INTERNAL
	  template<class Node, bool multiple>
	  class RBTreeNodeBaseImpl;

	  template<class Node>
	  class RBTreeNodeBaseImpl<Node, true> {
	  public:
		  enum class Color { RED, BLACK };
		  constexpr static bool _rbt_multiple = true;

		  Node *                      _rbt_parent = nullptr;
		  Node *                      _rbt_left = nullptr;
		  Node *                      _rbt_right = nullptr;
		  RBTreeNodeBaseImpl::Color   _rbt_color;

		  Node *                      _rbt_prev;
		  Node *                      _rbt_next;
	  };

	  template<class Node>
	  class RBTreeNodeBaseImpl<Node, false> {
	  public:
		  enum class Color { RED, BLACK };
		  constexpr static bool _rbt_multiple = false;

		  Node *                      _rbt_parent = nullptr;
		  Node *                      _rbt_left = nullptr;
		  Node *                      _rbt_right = nullptr;
		  RBTreeNodeBaseImpl::Color   _rbt_color;
	  };
	  /// @endcond
  } // namespace utilities


/**
 * @brief Base class (template) to supply your node class with metainformation
 *
 * The class you use as nodes for the Red-Black Tree *must* derive from this
 * class (template). It supplies your class with the necessary members to
 * contain the linking between the tree nodes.
 *
 * @tparam Node    The node class itself. Yes, that's the class derived from this template. This
 * sounds weird, but is correct. See the examples if you're confused.
 * @tparam options  The options class (a version of TreeOptions) that you parameterize the tree
 * with. (See the options parameter of RBTree.)
 */
template<class Node, class Options = TreeOptions<TreeFlags::MULTIPLE>>
class RBTreeNodeBase : public utilities::RBTreeNodeBaseImpl<Node, Options::multiple> {};

/**
 * @brief   Helper base class for the NodeTraits you need to implement
 *
 * This class serves as an (optional) base class for the NodeTraits you need to
 * implement. See DOCTODO for details on NodeTraits. This class just implements the
 * various hooks as empty functions.
 *
 * @tparam Node   The node class of your Red Black Tree
 */
template<class Node>
class RBDefaultNodeTraits {
public:
  static void leaf_inserted(Node & node) { (void)node; };
  static void fix_node(Node & node) { (void)node; };
  static void rotated_left(Node & node) { (void)node; };
  static void rotated_right(Node & node) { (void)node; };
  static void deleted_below(Node & node) { (void)node; };
  static void swapped(Node & n1, Node & n2) { (void)n1; (void)n2; };
};

/**
 * @brief The non-specialized parts of the Red-Black Tree
 *
 * This is the main Red-Black Tree class. For more information, see RBTree. This class cannot be
 * instantiated on its own.
 *
 */
template<class Node, class NodeTraits, class Compare = std::less<Node>>
class RBTreeBase
{
public:
  using Base = utilities::RBTreeNodeBaseImpl<Node, Node::_rbt_multiple>; // TODO rename
  using EqualityList = utilities::EqualityListHelper<Node, Node::_rbt_multiple, Compare>;

  /**
   * @brief Iterator over elements in the tree
   *
   * This class represents an iterator over elements in a Red-Black tree. The iterator
   * is an input iterator in terms of STL iterators, thus it provides only basic
   * functionality.
   *
   * *Warning*: For efficiency reasons, it is currently not possible to decrement the end() iterator!
   */
  template<bool reverse>
  class const_iterator {
  public:
    /// @cond INTERNAL
    typedef ptrdiff_t                         difference_type;
    typedef Node                              value_type;
    typedef const Node &                      const_reference;
    typedef const Node *                      const_pointer;
    typedef std::input_iterator_tag           iterator_category;

    const_iterator ();
    const_iterator (Node * n);
    const_iterator (const const_iterator & other);
    ~const_iterator();

    const_iterator& operator=(const const_iterator & other);
    const_iterator& operator=(const_iterator && other);

    bool operator==(const const_iterator & other) const;
    bool operator!=(const const_iterator & other) const;

    const_iterator& operator++();
    const_iterator  operator++(int);
    const_iterator& operator+=(size_t steps);
    const_iterator  operator+(size_t steps) const;

    const_iterator& operator--();
    const_iterator  operator--(int);

    const_reference operator*() const;
    const_pointer operator->() const;

    const_iterator<!reverse> get_reverse() const;

  private:
    Node * n;

    friend class utilities::IteratorHelperUnspecialized<Node, const_iterator<reverse>>;
    /// @endcond
  };

  /**
   * @brief Removes <node> from the tree
   *
   * Removes <node> from the tree.
   *
   * @param   Node  The node to be removed.
   */
  void remove(Node & node);

  /**
   * @brief Removes all elements from the tree.
   *
   * Removes all elements from the tree.
   */
  void clear();

  // Mainly debugging methods
  /// @cond INTERNAL
  bool verify_integrity() const;
  /// @endcond

  /**
   * @brief Debugging Method: Draw the Tree as a .dot file
   *
   * Outputs the current tree as a .dot file which can be drawn using
   * graphviz.
   *
   * @param filename  The file path where to write the .dot file.
   */
  void dump_to_dot(const std::string & filename) const;

  // Iteration
  /**
   * Returns an iterator pointing to the smallest element in the tree.
   */
  const_iterator<false> cbegin() const;
  /**
   * Returns an iterator pointing after the largest element in the tree.
   */
  const_iterator<false> cend() const;
  /**
   * Returns an iterator pointing to the smallest element in the tree.
   */
  const_iterator<false> begin() const;
  /**
   * Returns an iterator pointing after the largest element in the tree.
   */
  const_iterator<false> end() const;

  /**
   * Returns an reverse iterator pointing to the largest element in the tree.
   */
  const_iterator<true> crbegin() const;
  /**
   * Returns an reverse iterator pointing before the smallest element in the tree.
   */
  const_iterator<true> crend() const;
  /**
   * Returns an reverse iterator pointing to the largest element in the tree.
   */
  const_iterator<true> rbegin() const;
  /**
   * Returns an reverse iterator pointing before the smallest element in the tree.
   */
  const_iterator<true> rend() const;

  /**
   * Returns an iterator pointing to the entry held in node.
   *
   * @param node  The node the iterator should point to.
   */
  const_iterator<false> iterator_to(const Node & node) const;

protected:
	/**
 	 * This class should never be instantiated directly, only through RBTree!
 	 */
	RBTreeBase();


	Node *root;

  template<class NodeNameGetter>
  void dump_to_dot_base(const std::string & filename, NodeNameGetter name_getter) const;

  template<class NodeNameGetter>
  void output_node_base(const Node * node, std::ofstream & out, NodeNameGetter name_getter) const;

  using Path = std::vector<Node *>;

  Node * get_smallest() const;
  Node * get_largest() const;

  void remove_to_leaf(Node & node);
  void fixup_after_delete(Node * parent, bool deleted_left);

  template<bool on_equality_prefer_left>
  void insert_leaf_base(Node & node, Node * start);
  void insert_leaf(Node & node, Node * start);
  void insert_leaf_right_biased(Node & node, Node * start);

  void fixup_after_insert(Node * node);
  void rotate_left(Node * parent);
  void rotate_right(Node * parent);

  Node * get_uncle(Node * node) const;

  void swap_nodes(Node * n1, Node * n2, bool swap_colors = true);
  void swap_unrelated_nodes(Node * n1, Node * n2);
  void swap_neighbors(Node * parent, Node * child);

  bool verify_black_root() const;
  bool verify_black_paths(const Node * node, unsigned int * path_length) const;
  bool verify_red_black(const Node * node) const;
  bool verify_tree() const;
  bool verify_order() const;
  bool verify_equality() const;
};


/**
 * @brief The Red-Black Tree
 *
 * This is the main Red-Black Tree class.
 *
 * @tparam Node         The node class for this tree. It must be derived from RBTreeNodeBase.
 * @tparam NodeTraits   A class implementing various hooks and functions on your node class. See
 * DOCTODO for details.
 * @tparam Options			The TreeOptions class specifying the parameters of this RBTree. See the
 * TreeOptions and TreeFlags classes for details.
 * @tparam Compare      A compare class. The Red-Black Tree follows STL semantics for 'Compare'.
 * Defaults to std::less<Node>. Implement operator<(const Node & lhs, const Node & rhs) if you want to use it.
 */
template<class Node, class NodeTraits, class Options = TreeOptions<TreeFlags::MULTIPLE>,
         class Compare = std::less<Node>>
class RBTree : public RBTreeBase<Node, NodeTraits, Compare> {
public:
	RBTree();

	template<bool reverse>
	using const_iterator = typename RBTreeBase<Node, NodeTraits, Compare>::
																										template const_iterator<reverse>;
	using EqualityList = typename RBTreeBase<Node, NodeTraits, Compare>::EqualityList;

	/**
   * @brief Inserts <node> into the tree
   *
   * Inserts <node> into the tree.
   *
   * *Warning*: Please note that after calling insert() on a node (and before
   * removing that node again), that node *may not move in memory*. A common
   * pitfall is to store nodes in a std::vector (or other STL container), which
   * reallocates (and thereby moves objecs around).
   *
   * @warning Not available for explicitly ordered trees
   *
   * @param   Node  The node to be inserted.
   */
	void insert(Node & node);
	void insert(Node & node, Node & hint);
	void insert(Node & node, const_iterator<false> hint);

	/**
	 * @brief Finds an element in the tree
	 *
	 * Returns an iterator to the first element that compares equally to <query>.
	 * Note that <query> does not have to be a Node, but can be anything that can
	 * be compared to a Node, i.e., for which
	 *    Compare()(const Node &, const Comparable &)
	 * and
	 *    Compare()(const Comparable &, const Node &)
	 * are defined and implemented. In the case of using the default std::less as
	 * Compare, that means you have to implement operator<() for both types.
	 *
	 * @warning Not available for explicitly ordered trees
	 *
	 * @param query An object comparing equally to the element that should be found.
	 * @returns An iterator to the first element comparing equally to <query>, or end() if no such element exists
	 */
	template<class Comparable>
	const_iterator<false> find(const Comparable & query) const;

	/**
	 * @brief Upper-bounds an element
	 *
	 * Returns an iterator to the smallest element to which <query> compares as
	 * "less", i.e. that is considered to go after <query>.
	 *
	 * Note that <query> does not have to be a Node, but can be anything that can
	 * be compared to a Node, i.e., for which
	 *    Compare()(const Node &, const Comparable &)
	 * and
	 *    Compare()(const Comparable &, const Node &)
	 * are defined and implemented. In the case of using the default std::less as
	 * Compare, that means you have to implement operator<() for both types.
 	 *
   * @warning Not available for explicitly ordered trees
   *
	 * @param query An object comparable to Node that should be upper-bounded
	 * @returns An iterator to the first element comparing equally to <query>, or end() if no such element exists
	 */
	template<class Comparable>
	const_iterator<false> upper_bound(const Comparable & query) const;
};

#include "rbtree.cpp"

} // namespace ygg

#endif // RBTREE_HPP
