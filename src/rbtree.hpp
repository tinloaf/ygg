#ifndef RBTREE_HPP
#define RBTREE_HPP

#include <cstddef>

// Only for debugging purposes
#include <fstream>
#include <vector>

namespace ygg {
  namespace utilities {
    template<class Node, bool multiple, class Compare>
    class EqualityListHelper {};

    template<class Node, class Compare>
    class EqualityListHelper<Node, true, Compare> {
    public:
      static void equality_list_insert_node(Node & node, Node * predecessor);
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
      static void equality_list_insert_node(Node & node, Node * predecessor);
      static void equality_list_delete_node(Node & node);
      static Node * equality_list_find_first(Node * node);
      static Node * equality_list_next(Node * node);
      static Node * equality_list_prev(Node * node);
      static void equality_list_swap_if_necessary(Node & n1, Node & n2);
      static bool verify(const Node & n);
    };
  } // namespace utilities


/**
 * @brief Base class (template) to supply your node class with metainformation
 *
 * The class you use as nodes for the Red-Black Tree *must* derive from this
 * class (template). It supplies your class with the necessary members to
 * contain the linking between the tree nodes.
 *
 * @tparam Node       The node class itself. Yes, that's the class derived from this template. This sounds weird, but is correct. See the examples if you're confused.
 * @tparam multiple   Boolean specifying whether the Red-Black Tree should support multiple elements that compare equally to each other. See RBTree documentation for details.
 */
template<class Node, bool multiple = false>
class RBTreeNodeBase;

/// @cond INTERNAL
template<class Node>
class RBTreeNodeBase<Node, true> {
public:
  enum class Color { RED, BLACK };

  Node *                  _rbt_parent = nullptr;
  Node *                  _rbt_left = nullptr;
  Node *                  _rbt_right = nullptr;
  RBTreeNodeBase::Color   _rbt_color;

  Node *                  _rbt_prev;
  Node *                  _rbt_next;
};

template<class Node>
class RBTreeNodeBase<Node, false> {
public:
  enum class Color { RED, BLACK };

  Node *                  _rbt_parent = nullptr;
  Node *                  _rbt_left = nullptr;
  Node *                  _rbt_right = nullptr;
  RBTreeNodeBase::Color   _rbt_color;
};
/// @endcond

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
  static void leaf_inserted(Node & node) {};
  static void fix_node(Node & node) {};
  static void rotated_left(Node & node) {};
  static void rotated_right(Node & node) {};
  static void deleted_below(Node & node) {};
  static void swapped(Node & n1, Node & n2) {};
};

/**
 * @brief The Red-Black Tree
 *
 * This is the main Red-Black Tree class.
 *
 * @tparam Node         The node class for this tree. It must be derived from RBTreeNodeBase. DOCTODO link
 * @tparam NodeTraits   A class implementing various hooks and functions on your node class. See DOCTODO for details.
 * @tparam multiple     A boolean specifying whether multiple elements that compare equally to each other (i.e. with the same key) may be inserted into the tree. If you set this to false and nonetheless insert multiple equal elements, undefined behaviour occurrs. However, if you know that this will not happen, setting this to false will speed up operations and save a little memory.
 * @tparam Compare      A compare class. The Red-Black Tree follows STL semantics for 'Compare'. Defaults to std::less<Node>. Implement operator<(const Node & lhs, const Node & rhs) if you want to use it.
 */
template<class Node, class NodeTraits, bool multiple = false, class Compare = std::less<Node>>
class RBTree
{
public:
  using Base = RBTreeNodeBase<Node, multiple>;
  using EqualityList = utilities::EqualityListHelper<Node, multiple, Compare>;

  RBTree();

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
   * @param   Node  The node to be inserted.
   */
  void insert(Node & node);

  /**
   * @brief Removes <node> from the tree
   *
   * Removes <node> from the tree.
   *
   * @param   Node  The node to be removed.
   */
  void remove(Node & node);

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
  void dump_to_dot(std::string & filename) const;

  // Iteration
  /**
   * @brief Iterator over elements in the tree
   *
   * This class represents an iterator over elements in a Red-Black tree. The iterator
   * is an input iterator in terms of STL iterators, thus it provides only basic
   * functionality.
   */
  class const_iterator {
  public:
    /// @cond INTERNAL
    typedef ptrdiff_t                         difference_type;
    typedef Node                              value_type;
    typedef const Node &                      const_reference;
    typedef const Node *                      const_pointer;
    typedef std::input_iterator_tag           iterator_category;

    const_iterator ();
    const_iterator (Node * n, bool reverse = false);
    const_iterator (const const_iterator & other);
    ~const_iterator();

    const_iterator& operator=(const const_iterator & other);
    const_iterator& operator=(const_iterator && other);

    bool operator==(const const_iterator & other) const;
    bool operator!=(const const_iterator & other) const;

    const_iterator& operator++();
    const_iterator  operator++(int);
    const_iterator& operator+=(size_t n);
    const_iterator  operator+(size_t n) const;

    const_reference operator*() const;
    const_pointer operator->() const;

  private:
    Node * n;
    bool reverse;
    /// @endcond
  };

  /**
   * Returns an iterator pointing to the smallest element in the tree.
   */
  const_iterator cbegin() const;
  /**
   * Returns an iterator pointing after the largest element in the tree.
   */
  const_iterator cend() const;
  /**
   * Returns an iterator pointing to the smallest element in the tree.
   */
  const_iterator begin() const;
  /**
   * Returns an iterator pointing after the largest element in the tree.
   */
  const_iterator end() const;

  // querying for contained elements
  template<class Comparable>
  const_iterator find(const Comparable & query) const;

protected:
  Node *root;

  template<class NodeNameGetter>
  void dump_to_dot_base(std::string & filename, NodeNameGetter name_getter) const;

  template<class NodeNameGetter>
  void output_node_base(const Node * node, std::ofstream & out, NodeNameGetter name_getter) const;

private:
  using Path = std::vector<Node *>;

  void remove_to_leaf(Node & node);
  void fixup_after_delete(Node * parent, bool deleted_left);

  void insert_leaf(Node & node);

  void fix_upwards(Node * node);
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

#include "rbtree.cpp"

} // namespace ygg

#endif // RBTREE_HPP
