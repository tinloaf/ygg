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
  ZTreeRankFromHash() {};

  static int
  get_rank(const Node & node) noexcept
  {
    return __builtin_ffs(std::hash<Node>{}(node));
  }
};

template <class Node, class RankType>
class ZTreeRankFromHash<Node, RankType, false> {
public:
  ZTreeRankFromHash() {}; // TODO FIXME randomize!

  static int
  get_rank(const Node & node) noexcept
  {
    return node._zt_rank.rank;
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

  template<class NodeNameGetter>
  void dump_to_dot_base(const std::string & filename, NodeNameGetter name_getter) const;

  template<class NodeNameGetter>
  void output_node_base(const Node * node, std::ofstream & out, NodeNameGetter name_getter) const;

};

} // namespace ygg

#include "ziptree.cpp"

#endif
