#ifndef TEST_ZIPTREE_HPP
#define TEST_ZIPTREE_HPP

#include <algorithm>
#include <gtest/gtest.h>
#include <random>
#include <vector>

#include "../src/ziptree.hpp"

#define ZIPTREE_TESTSIZE 2000

namespace test {
namespace ziptree {

using namespace ygg;

class Node : public ZTreeNodeBase<Node> {
public:
  int data;
  int rank;

  Node() : data(0), rank(0){};
  Node(int data_in, int rank_in) : data(data_in), rank(rank_in){};
  Node(const Node & other) : data(other.data), rank(other.rank){};

  bool
  operator<(const Node & other) const
  {
    return this->data < other.data;
  }

  int
  get_data() const noexcept
  {
    return this->data;
  }

  int
  get_rank() const noexcept
  {
    return this->rank;
  }
};

class NodeTraits {
public:
  static std::string
  get_id(const Node * node)
  {
    return std::to_string(node->get_data()) + std::string("@") +
           std::to_string(node->get_rank());
  }
};

class DataRankGetter {
public:
  static size_t
  get_rank(const Node & n)
  {
    return n.get_rank();
  }
};

} // namespace ziptree
} // namespace test

namespace std {
template <>
struct hash<test::ziptree::Node>
{
  size_t
  operator()(const test::ziptree::Node & n) const noexcept
  {
    return hash<int>{}(n.get_data());
  }
};

} // namespace std

namespace test {
namespace ziptree {

TEST(ZipTreeTest, TrivialInsertionTest)
{
  auto tree = ZTree<Node, NodeTraits>();

  Node n(0,0);
  tree.insert(n);
  tree.dbg_verify();
}

TEST(ZipTreeTest, TrivialUnzippingTest)
{
  auto tree = ZTree<Node, NodeTraits, DefaultOptions, int,
                    ygg::rbtree_internal::flexible_less, DataRankGetter>();

  Node n0(0,1);
  Node n1(1,2);
  Node n2(2,0);
  Node n3(3,1);

  tree.insert(n0);
  tree.dump_to_dot("/tmp/dots/after-0.dot");
  tree.dbg_verify();

  tree.insert(n1);
  tree.dump_to_dot("/tmp/dots/after-1.dot");
  tree.dbg_verify();

  tree.insert(n2);
  tree.dump_to_dot("/tmp/dots/after-2.dot");
  tree.dbg_verify();

  tree.insert(n3);
  tree.dump_to_dot("/tmp/dots/after-3.dot");
  tree.dbg_verify();
}

TEST(ZipTreeTest, TrivialZippingTest)
{
  auto tree = ZTree<Node, NodeTraits, DefaultOptions, int,
                    ygg::rbtree_internal::flexible_less, DataRankGetter>();

  Node n0(0,1);
  Node n1(1,2);
  Node n2(2,0);
  Node n3(3,1);

  tree.insert(n0);
  tree.insert(n1);
  tree.insert(n2);
  tree.insert(n3);

  tree.dump_to_dot("/tmp/dots/before-delete-1.dot");

  tree.remove(n1);
  tree.dump_to_dot("/tmp/dots/after-delete-1.dot");
  tree.dbg_verify();

  tree.remove(n3);
  tree.dump_to_dot("/tmp/dots/after-delete-3.dot");
  tree.dbg_verify();

  tree.remove(n2);
  tree.dump_to_dot("/tmp/dots/after-delete-2.dot");
  tree.dbg_verify();

  tree.remove(n0);
  tree.dump_to_dot("/tmp/dots/after-delete-0.dot");
  tree.dbg_verify();
}

  
} // namespace ziptree
} // namespace test

#endif // TEST_ZIPTREE_HPP
