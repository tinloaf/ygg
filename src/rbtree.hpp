#ifndef RBTREE_HPP
#define RBTREE_HPP

#include <cstddef>

// Only for debugging purposes
#include <fstream>
#include <vector>

namespace iitree {
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
  }
}



template<class Node, bool multiple = false>
class RBTreeNodeBase;

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

template<class Node, class NodeTraits, bool multiple = false, class Compare = std::less<Node>>
class RBTree
{
public:
  using Base = RBTreeNodeBase<Node, multiple>;
  using EqualityList = iitree::utilities::EqualityListHelper<Node, multiple, Compare>;

  RBTree();

  // modifying the tree
  void insert(Node & node);
  void remove(Node & node);

  // Mainly debugging methods
  bool verify_integrity() const;
  void dump_to_dot(std::string & filename) const;

  // Iteration
  class const_iterator {
  public:
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
  };
  const_iterator cbegin() const;
  const_iterator cend() const;
  const_iterator begin() const;
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

#endif // RBTREE_HPP
