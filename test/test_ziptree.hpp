#ifndef TEST_ZIPTREE_HPP
#define TEST_ZIPTREE_HPP

#include <algorithm>
#include <gtest/gtest.h>
#include <random>
#include <vector>

#include "../src/ziptree.hpp"
#include "randomizer.hpp"

namespace ygg {
namespace testing {
namespace ziptree {

using namespace ygg;

constexpr size_t ZIPTREE_TESTSIZE = 2000;
constexpr size_t ZIPTREE_SEED = 4;

using ExplicitRankOptions =
    ygg::TreeOptions<TreeFlags::MULTIPLE, TreeFlags::CONSTANT_TIME_SIZE,
                     TreeFlags::ZTREE_RANK_TYPE<int>>;
using ImplicitRankOptions =
    ygg::TreeOptions<TreeFlags::MULTIPLE, TreeFlags::CONSTANT_TIME_SIZE,
                     TreeFlags::ZTREE_USE_HASH,
                     TreeFlags::ZTREE_RANK_TYPE<size_t>>;

class Node : public ZTreeNodeBase<Node, ExplicitRankOptions> {
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
    return (size_t)n.get_rank();
  }
};

class HashRankNode : public ZTreeNodeBase<HashRankNode, ImplicitRankOptions> {
public:
  int data;

  HashRankNode() : data(0){};
  HashRankNode(int data_in) : data(data_in){};
  HashRankNode(const Node & other) : data(other.data){};

  bool
  operator<(const HashRankNode & other) const
  {
    return this->data < other.data;
  }

  int
  get_data() const noexcept
  {
    return this->data;
  }
};

class HashRankNodeTraits {
public:
  static std::string
  get_id(const HashRankNode * node)
  {
    return std::to_string(node->get_data());
  }
};

} // namespace ziptree
} // namespace testing
} // namespace ygg

namespace std {
template <>
struct hash<ygg::testing::ziptree::HashRankNode>
{
  size_t
  operator()(const ygg::testing::ziptree::HashRankNode & n) const noexcept
  {
    return hash<int>{}(n.get_data());
  }
};

} // namespace std

namespace ygg {
namespace testing {
namespace ziptree {

using ExplicitRankTree =
    ZTree<Node, NodeTraits, ExplicitRankOptions, int,
          ygg::rbtree_internal::flexible_less, DataRankGetter>;
using ImplicitRankTree =
    ZTree<HashRankNode, HashRankNodeTraits, ImplicitRankOptions>;

TEST(ZipTreeTest, TrivialInsertionTest)
{
  ExplicitRankTree tree;

  Node n(0, 0);
  tree.insert(n);
  tree.dbg_verify();

  ImplicitRankTree itree;

  HashRankNode in(0);
  itree.insert(in);
  itree.dbg_verify();
}

TEST(ZipTreeTest, TrivialUnzippingTest)
{
  ExplicitRankTree tree;
  ImplicitRankTree itree;

  Node n0(0, 1);
  Node n1(1, 2);
  Node n2(2, 0);
  Node n3(3, 1);

  HashRankNode in0(32);
  HashRankNode in1(36);
  HashRankNode in2(2);
  HashRankNode in3(3);

  tree.insert(n0);
  tree.dump_to_dot("/tmp/dots/after-0.dot");
  tree.dbg_verify();

  itree.insert(in0);
  itree.dbg_verify();

  tree.insert(n1);
  tree.dump_to_dot("/tmp/dots/after-1.dot");
  tree.dbg_verify();

  itree.insert(in1);
  itree.dbg_verify();

  tree.insert(n2);
  tree.dump_to_dot("/tmp/dots/after-2.dot");
  tree.dbg_verify();

  itree.insert(in2);
  itree.dbg_verify();

  tree.insert(n3);
  tree.dump_to_dot("/tmp/dots/after-3.dot");
  tree.dbg_verify();

  itree.insert(in3);
  itree.dbg_verify();
}

TEST(ZipTreeTest, TrivialZippingTest)
{
  ExplicitRankTree tree;
  ImplicitRankTree itree;

  Node n0(0, 1);
  Node n1(1, 2);
  Node n2(2, 0);
  Node n3(3, 1);

  HashRankNode in0(0);
  HashRankNode in1(1);
  HashRankNode in2(2);
  HashRankNode in3(3);

  tree.insert(n0);
  tree.insert(n1);
  tree.insert(n2);
  tree.insert(n3);
  itree.insert(in0);
  itree.insert(in1);
  itree.insert(in2);
  itree.insert(in3);

  tree.dump_to_dot("/tmp/dots/before-delete-1.dot");

  tree.remove(n1);
  tree.dump_to_dot("/tmp/dots/after-delete-1.dot");
  tree.dbg_verify();
  itree.remove(in1);
  itree.dbg_verify();

  tree.remove(n3);
  tree.dump_to_dot("/tmp/dots/after-delete-3.dot");
  tree.dbg_verify();
  itree.remove(in3);
  itree.dbg_verify();

  tree.remove(n2);
  tree.dump_to_dot("/tmp/dots/after-delete-2.dot");
  tree.dbg_verify();
  itree.remove(in2);
  itree.dbg_verify();

  tree.remove(n0);
  tree.dump_to_dot("/tmp/dots/after-delete-0.dot");
  tree.dbg_verify();
  itree.remove(in0);
  itree.dbg_verify();
}

TEST(ZipTreeTest, InsertionAndIterationTest)
{
  ExplicitRankTree tree;
  ImplicitRankTree itree;

  Node nodes[ZIPTREE_TESTSIZE];
  HashRankNode inodes[ZIPTREE_TESTSIZE];

  std::vector<size_t> indices;

  for (size_t i = 0; i < ZIPTREE_TESTSIZE; ++i) {
    nodes[i] = Node((int)i, (int)i);
    inodes[i] = HashRankNode((int)i);

    indices.push_back(i);
  }

  std::shuffle(indices.begin(), indices.end(),
               ygg::testing::utilities::Randomizer(ZIPTREE_SEED));

  for (auto index : indices) {
    tree.insert(nodes[index]);
    itree.insert(inodes[index]);
    tree.dbg_verify();
    itree.dbg_verify();
  }

  int i = 0;
  for (auto & node : tree) {
    ASSERT_EQ(node.get_data(), i);
    ASSERT_EQ(node.get_rank(), i);
    i++;
  }

  i = 0;
  for (auto & node : itree) {
    ASSERT_EQ(node.get_data(), i);
    i++;
  }
}

TEST(ZipTreeTest, InsertionAndDeletionTest)
{
  ExplicitRankTree tree;
  ImplicitRankTree itree;

  Node nodes[ZIPTREE_TESTSIZE];
  HashRankNode inodes[ZIPTREE_TESTSIZE];

  std::vector<size_t> indices;
  std::vector<size_t> remove_indices;

  for (size_t i = 0; i < ZIPTREE_TESTSIZE; ++i) {
    nodes[i] = Node((int)i, (int)i);
    inodes[i] = HashRankNode((int)i);

    indices.push_back(i);
    remove_indices.push_back(i);
  }

  std::shuffle(indices.begin(), indices.end(),
               ygg::testing::utilities::Randomizer(ZIPTREE_SEED));
  std::shuffle(remove_indices.begin(), remove_indices.end(),
               ygg::testing::utilities::Randomizer(ZIPTREE_SEED + 1));

  size_t i = 0;
  for (auto index : indices) {
    ASSERT_EQ(i, tree.size());
    tree.insert(nodes[index]);
    itree.insert(inodes[index]);
    i++;
  }

  tree.dbg_verify();
  itree.dbg_verify();

  for (auto index : remove_indices) {
    tree.remove(nodes[index]);
    itree.remove(inodes[index]);

    i--;
    ASSERT_EQ(i, tree.size());

    tree.dbg_verify();
    itree.dbg_verify();
  }

  ASSERT_TRUE(tree.empty());
  auto it = tree.begin();
  ASSERT_TRUE(it == tree.end());
  auto iit = itree.begin();
  ASSERT_TRUE(iit == itree.end());
}

TEST(ZipTreeTest, ComprehensiveTest)
{
  ExplicitRankTree tree;
  ImplicitRankTree itree;

  Node persistent_nodes[ZIPTREE_TESTSIZE];
  HashRankNode ipersistent_nodes[ZIPTREE_TESTSIZE];

  std::vector<unsigned int> indices;
  std::mt19937 rng(ZIPTREE_SEED);

  std::set<unsigned int> values_seen;

  for (unsigned int i = 0; i < ZIPTREE_TESTSIZE; ++i) {
    unsigned int data = 10 * i;
    persistent_nodes[i] = Node((int)data, (int)data);
    ipersistent_nodes[i] = HashRankNode((int)data);

    indices.push_back(i);
    values_seen.insert(data);
  }

  std::shuffle(indices.begin(), indices.end(),
               ygg::testing::utilities::Randomizer(ZIPTREE_SEED));

  for (auto index : indices) {
    tree.insert(persistent_nodes[index]);
    itree.insert(ipersistent_nodes[index]);
  }

  tree.dbg_verify();
  itree.dbg_verify();

  Node transient_nodes[ZIPTREE_TESTSIZE];
  HashRankNode itransient_nodes[ZIPTREE_TESTSIZE];

  for (unsigned int i = 0; i < ZIPTREE_TESTSIZE; ++i) {
    std::uniform_int_distribution<unsigned int> uni(
        0, 10 * (ZIPTREE_TESTSIZE + 1));
    unsigned int data = uni(rng);

    while (values_seen.find(data) != values_seen.end()) {
      data = uni(rng);
    }

    transient_nodes[i] = Node((int)data, (int)data);
    itransient_nodes[i] = HashRankNode((int)data);

    values_seen.insert(data);
  }

  std::shuffle(indices.begin(), indices.end(),
               ygg::testing::utilities::Randomizer(ZIPTREE_SEED));

  for (auto index : indices) {
    tree.insert(transient_nodes[index]);
    itree.insert(itransient_nodes[index]);
  }

  tree.dbg_verify();
  itree.dbg_verify();

  for (int i = 0; i < ZIPTREE_TESTSIZE; ++i) {
    tree.remove(transient_nodes[i]);
    itree.remove(itransient_nodes[i]);

    tree.dbg_verify();
    itree.dbg_verify();
  }

  // Query elements
  /*
  for (int i = 0; i < ZIPTREE_TESTSIZE; ++i) {
    auto it = tree.find(persistent_nodes[i]);
    assert(&(*it) == &(persistent_nodes[i]));
    ASSERT_EQ(&(*it), &(persistent_nodes[i]));
  }
  */
}


/*****************************************
 * Test for individual bugs
 *****************************************/
TEST(ZipTreeTest, Bug1Test)
{

  ImplicitRankTree itree;

  HashRankNode nodes[] = {
    {40}, {0}, {10}, {30}, {20}, {47}, {26}, {4}, {19}, {13}
  };

  for (auto & node : nodes) {
    itree.insert(node);
    itree.dbg_verify();
  }

  itree.dump_to_dot("/tmp/dots/bug1-before-first-remove.dot");
  itree.remove(nodes[7]); // 4
  itree.dump_to_dot("/tmp/dots/bug1-after-first-remove.dot");
  itree.dbg_verify();
  itree.remove(nodes[9]); // 13
  itree.dbg_verify();
  itree.remove(nodes[5]); // 47
  itree.dbg_verify();
  itree.dump_to_dot("/tmp/dots/bug1-before.dot");
  itree.remove(nodes[8]); // 19
  itree.dump_to_dot("/tmp/dots/bug1-after.dot");
  itree.dbg_verify();
}


} // namespace ziptree
} // namespace testing
} // namespace ygg

#endif // TEST_ZIPTREE_HPP
