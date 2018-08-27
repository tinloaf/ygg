#pragma once
#ifndef YGG_ZIPTREE_H
#define YGG_ZIPTREE_H

#include "options.hpp"
#include "size_holder.hpp"

#include <functional>

namespace ygg {

// Forward
template <class Node, class Options = DefaultOptions, class Tag = int>
class ZTreeNodeBase;

namespace ztree_internal {

template <class Tree, bool enable>
struct dbg_verify_size_helper
{
  void operator()(const Tree & t, size_t node_count);
};

template <class Tree>
struct dbg_verify_size_helper<Tree, true>
{
  void
  operator()(const Tree & t, size_t node_count)
  {
    assert(t.size() == node_count);
    // Usage in assertion is not enough for GCC to realize that the variable is
    // being used.
    (void)t;
    (void)node_count;
  }
};

template <class Tree>
struct dbg_verify_size_helper<Tree, false>
{
  void
  operator()(const Tree & t, size_t node_count)
  {}
};

template <class Node, class RankType, bool use_hash>
class ZTreeRankFromHash;

template <class Node, class RankType>
class ZTreeRankFromHash<Node, RankType, true> {
public:
  ZTreeRankFromHash(){};

  static int
  get_rank(const Node & node) noexcept
  {
    // TODO ffsl? ffs?
    return __builtin_ffsl((long int)std::hash<Node>{}(node));
  }
};

template <class Node, class RankType>
class ZTreeRankFromHash<Node, RankType, false> {
public:
  ZTreeRankFromHash(){}; // TODO FIXME randomize!

  // TODO is int the right type here?
  static int
  get_rank(const Node & node) noexcept
  {
    return (int)node._zt_rank.rank;
  }

private:
  template <class, class, class>
  friend class ZTreeNodeBase;
  RankType rank;
};

} // namespace ztree_internal

template <class Node, class Options, class Tag>
class ZTreeNodeBase {
public:
  Node * _zt_parent = nullptr;
  Node * _zt_left = nullptr;
  Node * _zt_right = nullptr;

private:
  friend class ztree_internal::ZTreeRankFromHash<
      Node, typename Options::ztree_rank_type, Options::ztree_use_hash>;

  ztree_internal::ZTreeRankFromHash<Node, typename Options::ztree_rank_type,
                                    Options::ztree_use_hash>
      _zt_rank;
};

// TODO NodeTraits
template <class Node, class NodeTraits, class Options = DefaultOptions,
          class Tag = int, class Compare = ygg::rbtree_internal::flexible_less,
          class RankGetter = ztree_internal::ZTreeRankFromHash<
              Node, typename Options::ztree_rank_type, Options::ztree_use_hash>>
class ZTree {
public:
  using NB = ZTreeNodeBase<Node, Options, Tag>;
  using my_type = ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>;

  ZTree() noexcept;
  static_assert(std::is_base_of<NB, Node>::value,
                "Node class not properly derived from node base!");

private:
  // Class to tell the abstract search tree iterator how to handle
  // our nodes
  class NodeInterface {
  public:
    static Node *
    get_parent(Node * n)
    {
      return n->NB::_zt_parent;
    }
    static Node *
    get_left(Node * n)
    {
      return n->NB::_zt_left;
    }
    static Node *
    get_right(Node * n)
    {
      return n->NB::_zt_right;
    }

    static const Node *
    get_parent(const Node * n)
    {
      return n->NB::_zt_parent;
    }
    static const Node *
    get_left(const Node * n)
    {
      return n->NB::_zt_left;
    }
    static const Node *
    get_right(const Node * n)
    {
      return n->NB::_zt_right;
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

  void insert(Node & node) noexcept;
  void insert(Node & node, Node & hint) noexcept;

  void remove(Node & node) noexcept;

  // TODO add rank-shortened search

  template <class Comparable>
  const_iterator<false> find(const Comparable & query) const;
  template <class Comparable>
  iterator<false> find(const Comparable & query);

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

  // Debugging methods
  void dbg_verify() const;

  /**
   * @brief Debugging Method: Draw the Tree as a .dot file
   *
   * Outputs the current tree as a .dot file which can be drawn using
   * graphviz.
   *
   * @param filename  The file path where to write the .dot file.
   */
  void dump_to_dot(const std::string & filename) const;

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

private:
  Node * root;
  Compare cmp;

  void unzip(Node & oldn, Node & newn) noexcept;
  Node * zip(Node & old_root) noexcept;

  Node * get_smallest() const;
  Node * get_largest() const;

  // Debugging methods
  void dbg_verify_consistency(Node * sub_root, Node * lower_bound,
                              Node * upper_bound) const;
  void dbg_verify_size() const;

  template <class NodeNameGetter>
  void dump_to_dot_base(const std::string & filename,
                        NodeNameGetter name_getter) const;

  template <class NodeNameGetter>
  void output_node_base(const Node * node, std::ofstream & out,
                        NodeNameGetter name_getter) const;

  SizeHolder<Options::constant_time_size> s;
};

} // namespace ygg

#include "ziptree.cpp"

#endif
