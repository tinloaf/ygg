#ifndef RBTREE_HPP
#define RBTREE_HPP

// Only for debugging purposes
#include <fstream>

template<class Node>
class RBTreeNodeBase {
public:
  enum class Color { RED, BLACK };

  Node *                  _rbt_parent = nullptr;
  Node *                  _rbt_left = nullptr;
  Node *                  _rbt_right = nullptr;
  RBTreeNodeBase::Color   _rbt_color;
};

template<class Node, class NodeTraits, class Compare = std::less<Node>>
class RBTree
{
public:
  using Base = RBTreeNodeBase<Node>;

  RBTree();

  // modifying the tree
  void insert(Node & node);
  void remove(Node & node);

  // Mainly debugging methods
  bool verify_integrity() const;
  void dump_to_dot(std::string filename) const;

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
    // TODO move constructor
    const_iterator (const const_iterator & other);
    ~const_iterator();

    const_iterator& operator=(const const_iterator & other);
    const_iterator& operator=(const_iterator && other);

    bool operator==(const const_iterator & other) const;
    bool operator!=(const const_iterator & other) const;

    const_iterator& operator++();
    const_iterator operator++(int); //optional
    const_iterator& operator+=(size_t n); //optional
    const_iterator operator+(size_t n) const; //optional

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

private:
  using Path = std::vector<Node *>;

  void add_path_to_smallest(Path & path, Node * root) const;

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

  Node *root;


  bool verify_black_root() const;
  bool verify_black_paths(const Node * node, unsigned int * path_length) const;
  bool verify_red_black(const Node * node) const;
  bool verify_tree() const;

  void output_node(const Node * node, std::ofstream & out) const;
};

#include "rbtree.cpp"

#endif // RBTREE_HPP
