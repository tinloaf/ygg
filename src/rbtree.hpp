#ifndef RBTREE_HPP
#define RBTREE_HPP

#include <cassert>
#include <cstddef>
#include <set>
#include <type_traits>

#include "options.hpp"
#include "size_holder.hpp"
#include "tree_iterator.hpp"

// Only for debugging purposes
#include <fstream>
#include <vector>

namespace ygg {
namespace rbtree_internal {
/// @cond INTERNAL

enum class Color
{
  RED,
  BLACK
};

template <class Node, bool compress_color>
class ColorParentStorage;

template <class Node>
class ColorParentStorage<Node, true> {
public:
  void set_color(Color new_color);
  Color get_color() const;
  void set_parent(Node * new_parent);
  Node * get_parent() const;

  void swap_parent_with(ColorParentStorage<Node, true> & other);
  void swap_color_with(ColorParentStorage<Node, true> & other);

private:
  Node * parent;
};

template <class Node>
class ColorParentStorage<Node, false> {
public:
  void set_color(Color new_color);
  Color get_color() const;
  void set_parent(Node * new_parent);
  Node * get_parent() const;

  void swap_parent_with(ColorParentStorage<Node, false> & other);
  void swap_color_with(ColorParentStorage<Node, false> & other);

private:
  Node * parent = nullptr;
  Color color;
};

template <class Node, class Tag, bool compress_color>
class RBTreeNodeBaseImpl {
public:
  Node * _rbt_left = nullptr;
  Node * _rbt_right = nullptr;

  ColorParentStorage<Node, compress_color> _color_and_parent;

  // TODO namespaceing!
  void set_color(Color new_color);
  Color get_color() const;
  void set_parent(Node * new_parent);
  Node * get_parent() const;

  void swap_parent_with(Node * other);
  void swap_color_with(Node * other);
};

/// @endcond
} // namespace rbtree_internal

// TODO document
template <class Node>
class DefaultFindCallbacks {
public:
  void
  init_root(Node * root)
  {
    (void)root;
  };
  void
  descend_left(Node * child)
  {
    (void)child;
  };
  void
  descend_right(Node * child)
  {
    (void)child;
  };
  void
  found(Node * node)
  {
    (void)node;
  };
  void not_found(){};

  static DefaultFindCallbacks<Node> dummy;
};

/**
 * @brief Base class (template) to supply your node class with metainformation
 *
 * The class you use as nodes for the Red-Black Tree *must* derive from this
 * class (template). It supplies your class with the necessary members to
 * contain the linking between the tree nodes.
 *
 * @tparam Node    The node class itself. Yes, that's the class derived from
 * this template. This sounds weird, but is correct. See the examples if you're
 * confused.
 * @tparam options  The options class (a version of TreeOptions) that you
 * parameterize the tree with. (See the options parameter of RBTree.)
 * @tparam Tag 		The tag used to identify the tree that this node should
 * be inserted into. See RBTree for details.
 */
template <class Node, class Options = DefaultOptions, class Tag = int>
class RBTreeNodeBase
    : public rbtree_internal::RBTreeNodeBaseImpl<Node, Tag,
                                                 Options::compress_color> {
};

/**
 * @brief   Helper base class for the NodeTraits you need to implement
 *
 * This class serves as an (optional) base class for the NodeTraits you need to
 * implement. See DOCTODO for details on NodeTraits. This class just implements
 * the various hooks as empty functions.
 *
 * @tparam Node   The node class of your Red Black Tree
 */
template <class Node>
class RBDefaultNodeTraits {
public:
  // TODO document
  static void
  leaf_inserted(Node & node)
  {
    (void)node;
  };
  static void
  rotated_left(Node & node)
  {
    (void)node;
  };
  static void
  rotated_right(Node & node)
  {
    (void)node;
  };
  static void
  delete_leaf(Node & node)
  {
    (void)node;
  };
  static void
  deleted_below(Node & node)
  {
    (void)node;
  };
  static void
  swapped(Node & old_ancestor, Node & old_descendant)
  {
    (void)old_ancestor;
    (void)old_descendant;
  };
};

/**
 * @brief The Red-Black Tree
 *
 * This is the main Red-Black Tree class.
 *
 * @tparam Node         The node class for this tree. It must be derived from
 * RBTreeNodeBase.
 * @tparam NodeTraits   A class implementing various hooks and functions on your
 * node class. See DOCTODO for details.
 * @tparam Options			The TreeOptions class specifying the
 * parameters of this RBTree. See the TreeOptions and TreeFlags classes for
 * details.
 * @tparam Tag					An class tag that identifies
 * this tree. Can be used to insert the same nodes into multiple trees. See
 * DOCTODO for details. Can be any class, the class can be empty
 * @tparam Compare      A compare class. The Red-Black Tree follows STL
 * semantics for 'Compare'. Defaults to ygg::utilities::flexible_less. Implement
 * operator<(const Node & lhs, const Node & rhs) if you want to use it.
 */
template <class Node, class NodeTraits, class Options = DefaultOptions,
          class Tag = int, class Compare = ygg::rbtree_internal::flexible_less>
class RBTree {
public:
  using Base =
      rbtree_internal::RBTreeNodeBaseImpl<Node, Tag,
                                          Options::compress_color>; // TODO
  // rename

  RBTree();

  // Node Base
  using NB = RBTreeNodeBase<Node, Options, Tag>;
  static_assert(std::is_base_of<NB, Node>::value,
                "Node class not properly derived from RBTreeNodeBase");

  // Class to tell the abstract search tree iterator how to handle our nodes
private:
  class NodeInterface {
  public:
    static Node *
    get_parent(Node * n)
    {
      return n->NB::get_parent();
    }
    static Node *
    get_left(Node * n)
    {
      return n->NB::_rbt_left;
    }
    static Node *
    get_right(Node * n)
    {
      return n->NB::_rbt_right;
    }

    static const Node *
    get_parent(const Node * n)
    {
      return n->NB::get_parent();
    }
    static const Node *
    get_left(const Node * n)
    {
      return n->NB::_rbt_left;
    }
    static const Node *
    get_right(const Node * n)
    {
      return n->NB::_rbt_right;
    }
  };

public:
  // forward, for friendship
  template <bool reverse>
  class const_iterator;

  template <bool reverse>
  class iterator : public internal::IteratorBase<iterator<reverse>, Node,
                                                 NodeInterface, reverse> {
  public:
    using internal::IteratorBase<iterator<reverse>, Node, NodeInterface,
                                 reverse>::IteratorBase;
    iterator(const iterator<reverse> & orig)
        : internal::IteratorBase<iterator<reverse>, Node, NodeInterface,
                                 reverse>(orig.n){};
    iterator()
        : internal::IteratorBase<iterator<reverse>, Node, NodeInterface,
                                 reverse>(){};

  private:
    friend class const_iterator<reverse>;
  };

  template <bool reverse>
  class const_iterator
      : public internal::IteratorBase<const_iterator<reverse>, const Node,
                                      NodeInterface, reverse> {
  public:
    using internal::IteratorBase<const_iterator<reverse>, const Node,
                                 NodeInterface, reverse>::IteratorBase;
    const_iterator(const const_iterator<reverse> & orig)
        : internal::IteratorBase<const_iterator<reverse>, const Node,
                                 NodeInterface, reverse>(orig.n){};
    const_iterator(const iterator<reverse> & orig)
        : internal::IteratorBase<const_iterator<reverse>, const Node,
                                 NodeInterface, reverse>(orig.n){};
    const_iterator()
        : internal::IteratorBase<const_iterator<reverse>, const Node,
                                 NodeInterface, reverse>(){};
  };

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
  void insert(Node & node, iterator<false> hint);

  // TODO document
  void insert_left_leaning(Node & node);
  void insert_right_leaning(Node & node);

  /**
   * @brief Finds an element in the tree
   *
   * Returns an iterator to the first element that compares equally to <query>.
   * Note that <query> does not have to be a Node, but can be anything that can
   * be compared to a Node, i.e., for which
   *    Compare()(const Node &, const Comparable &)
   * and
   *    Compare()(const Comparable &, const Node &)
   * are defined and implemented. In the case of using the default
   * ygg::utilities::flexible_less as Compare, that means you have to implement
   * operator<() for both types.
   *
   * @warning Not available for explicitly ordered trees
   *
   * @param query An object comparing equally to the element that should be
   * found.
   * @returns An iterator to the first element comparing equally to <query>, or
   * end() if no such element exists
   */
  template <class Comparable>
  const_iterator<false> find(const Comparable & query) const;
  template <class Comparable>
  iterator<false> find(const Comparable & query);

  // TODO document
  // TODO test
  template <class Comparable, class Callbacks = DefaultFindCallbacks<Node>>
  iterator<false> find(const Comparable & query, Callbacks * cbs);

  /**
   * @brief Upper-bounds an element
   *
   * Returns an iterator to the smallest element to which <query> compares as
   * "less", i.e. the smallest element that is considered go strictly after
   * <query>.
   *
   * Note that <query> does not have to be a Node, but can be anything that can
   * be compared to a Node, i.e., for which
   *    Compare()(const Node &, const Comparable &)
   * and
   *    Compare()(const Comparable &, const Node &)
   * are defined and implemented. In the case of using the default
   * ygg::utilities::flexible_less as Compare, that means you have to implement
   * operator<() for both types.
   *
   * @warning Not available for explicitly ordered trees
   *
   * @param query An object comparable to Node that should be upper-bounded
   * @returns An iterator to the first element comparing "greater" to <query>,
   * or end() if no such element exists
   */
  template <class Comparable>
  const_iterator<false> upper_bound(const Comparable & query) const;
  template <class Comparable>
  iterator<false> upper_bound(const Comparable & query);

  /**
   * @brief Lower-bounds an element
   *
   * Returns an iterator to the first element that is not less that <query>,
   * i.e., that does not have to go before <query>.
   *
   * Note that <query> does not have to be a Node, but can be anything that can
   * be compared to a Node, i.e., for which
   *    Compare()(const Node &, const Comparable &)
   * and
   *    Compare()(const Comparable &, const Node &)
   * are defined and implemented. In the case of using the default
   * ygg::utilities::flexible_less as Compare, that means you have to implement
   * operator<() for both types.
   *
   * @warning Not available for explicitly ordered trees
   *
   * @param query An object comparable to Node that should be lower-bounded
   * @returns An iterator to the first element comparing greater-or-equally to
   * <query>, or end() if no such element exists
   */
  template <class Comparable>
  const_iterator<false> lower_bound(const Comparable & query) const;
  template <class Comparable>
  iterator<false> lower_bound(const Comparable & query);

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
  iterator<false> begin();

  /**
   * Returns an iterator pointing after the largest element in the tree.
   */
  const_iterator<false> end() const;
  iterator<false> end();

  /**
   * Returns an reverse iterator pointing to the largest element in the tree.
   */
  const_iterator<true> crbegin() const;
  /**
   * Returns an reverse iterator pointing before the smallest element in the
   * tree.
   */
  const_iterator<true> crend() const;
  /**
   * Returns an reverse iterator pointing to the largest element in the tree.
   */
  const_iterator<true> rbegin() const;
  iterator<true> rbegin();

  /**
   * Returns an reverse iterator pointing before the smallest element in the
   * tree.
   */
  const_iterator<true> rend() const;
  iterator<true> rend();

  /**
   * Returns an iterator pointing to the entry held in node.
   *
   * @param node  The node the iterator should point to.
   */
  const_iterator<false> iterator_to(const Node & node) const;
  iterator<false> iterator_to(Node & node);

  /**
   * Return the number of elements in the tree.
   *
   * This method runs in O(1).
   *
   * @warning This method is only available if CONSTANT_TIME_SIZE is set as
   * option!
   *
   * @return The number of elements in the tree.
   */
  size_t size() const;

  /**
   * @brief Returns whether the tree is empty
   *
   * This method runs in O(1).
   *
   * @return true if the tree is empty, false otherwise
   */
  bool empty() const;

  // TODO document
  Node * get_root() const;
  static Node * get_parent(Node * n);
  static Node * get_left_child(Node * n);
  static Node * get_right_child(Node * n);

protected:
  Node * root;

  template <class NodeNameGetter>
  void dump_to_dot_base(const std::string & filename,
                        NodeNameGetter name_getter) const;

  template <class NodeNameGetter>
  void output_node_base(const Node * node, std::ofstream & out,
                        NodeNameGetter name_getter) const;

  using Path = std::vector<Node *>;

  Node * get_smallest() const;
  Node * get_largest() const;

  void remove_to_leaf(Node & node);
  void fixup_after_delete(Node * parent, bool deleted_left);

  template <bool on_equality_prefer_left>
  void insert_leaf_base(Node & node, Node * start);

  void fixup_after_insert(Node * node);
  void rotate_left(Node * parent);
  void rotate_right(Node * parent);

  Node * get_uncle(Node * node) const;

  void swap_nodes(Node * n1, Node * n2, bool swap_colors = true);
  void replace_node(Node * to_be_replaced, Node * replace_with);
  void swap_unrelated_nodes(Node * n1, Node * n2);
  void swap_neighbors(Node * parent, Node * child);

  bool verify_black_root() const;
  bool verify_black_paths(const Node * node, unsigned int * path_length) const;
  bool verify_red_black(const Node * node) const;
  bool verify_tree() const;
  bool verify_order() const;

  Compare cmp;

  SizeHolder<Options::constant_time_size> s;
};

} // namespace ygg

#include "rbtree.cpp"

#endif // RBTREE_HPP
