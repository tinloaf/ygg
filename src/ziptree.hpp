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

template <class Node, class RankType, bool use_hash>
class ZTreeRankFromHash;

template <class Node, class RankType>
class ZTreeRankFromHash<Node, RankType, true> {
public:
  ZTreeRankFromHash(){};

  static int
  get_rank(const Node & node) noexcept
  {
    return __builtin_ffs(std::hash<Node>{}(node));
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

  ZTree() noexcept;
  static_assert(std::is_base_of<NB, Node>::value,
                "Node class not properly derived from node base!");

private:
  // Class to tell the abstract search tree iterator how to handle
  // our nodes
  class NodeInterface {
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

private:
  Node * root;
  Compare cmp;

  void unzip(Node & oldn, Node & newn) noexcept;
  Node * zip(Node & old_root) noexcept;

  // Debugging methods
  void dbg_verify_consistency(Node * sub_root) const;

  template <class NodeNameGetter>
  void dump_to_dot_base(const std::string & filename,
                        NodeNameGetter name_getter) const;

  template <class NodeNameGetter>
  void output_node_base(const Node * node, std::ofstream & out,
                        NodeNameGetter name_getter) const;
};

} // namespace ygg

#include "ziptree.cpp"

#endif
