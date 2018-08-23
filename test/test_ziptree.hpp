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

  Node() : data(0){};
  explicit Node(int data_in) : data(data_in){};
  Node(const Node & other) : data(other.data){};

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
};

class NodeTraits {
public:
  static std::string get_id(const Node * node) {
    return std::to_string(node->get_data());
  }
};
  
class DataRankGetter {
public:
  static size_t get_rank(const Node & n) {
    return n.get_data();
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

  Node n;
  n.data = 0;
  tree.insert(n);
  tree.dbg_verify();
}

TEST(ZipTreeTest, TrivialUnzippingTest)
{
  auto tree = ZTree<Node, NodeTraits, DefaultOptions, int, ygg::rbtree_internal::flexible_less, DataRankGetter>();

  Node n0(0);
  Node n1(1);
  Node n2(2);
  Node n3(3);
  
  tree.insert(n0);
  tree.dump_to_dot("/tmp/dots/after-0.dot");
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

} // namespace ziptree
} // namespace test

#endif // TEST_ZIPTREE_HPP
