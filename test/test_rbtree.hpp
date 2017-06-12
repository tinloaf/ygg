#ifndef TEST_RBTREE_HPP
#define TEST_RBTREE_HPP

#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <algorithm>

#include "../src/rbtree.hpp"

#define TESTSIZE 5000

template<typename T> constexpr const T &as_const(T &t) noexcept { return t; }

class Node : public RBTreeNodeBase<Node> {
public:
  int data;

  Node () : data(0) {};
  explicit Node(int data_in) : data(data_in) {};
  Node(const Node &other) : data(other.data) {};

  bool operator<(const Node & other) const {
    return this->data < other.data;
  }
};

class NodeTraits {
public:
  static std::string get_id(const Node * node) {
    return std::to_string(node->data);
  }

  static void leaf_inserted(Node & node) { (void)node ; };
  static void rotated_left(Node & node) { (void)node ; };
  static void rotated_right(Node & node) { (void)node ; };
  static void deleted_below(Node & node) { (void)node ; };
  static void swapped(Node & n1, Node & n2) { (void)n1 ; (void)n2 ; };
};


TEST(RBTreeTest, TrivialInsertionTest) {
  auto tree = RBTree<Node, NodeTraits>();

  Node n;
  n.data = 0;
  tree.insert(n);

  ASSERT_TRUE(tree.verify_integrity());
}


TEST(RBTreeTest, RandomInsertionTest) {
  auto tree = RBTree<Node, NodeTraits>();

  std::mt19937 rng(4); // chosen by fair xkcd
  std::uniform_int_distribution<int> uni(std::numeric_limits<int>::min(),
                                         std::numeric_limits<int>::max());

  Node nodes[TESTSIZE];

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    nodes[i] = Node(uni(rng));

    tree.insert(nodes[i]);

    //std::string fname = std::string("/tmp/trees/tree-") + std::to_string(i) + std::string(".dot");
    //tree.dump_to_dot(fname);

    ASSERT_TRUE(tree.verify_integrity());
  }
}

TEST(RBTreeTest, LinearInsertionTest) {
  auto tree = RBTree<Node, NodeTraits>();

  Node nodes[TESTSIZE];

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    nodes[i] = Node(i);

    tree.insert(nodes[i]);

    //std::string fname = std::string("/tmp/trees/tree-") + std::to_string(i) + std::string(".dot");
    //tree.dump_to_dot(fname);

    ASSERT_TRUE(tree.verify_integrity());
  }
}

TEST(RBTreeTest, TrivialDeletionTest) {
  auto tree = RBTree<Node, NodeTraits>();

  Node n1;
  n1.data = 0;
  tree.insert(n1);

  Node n2;
  n2.data = 1;
  tree.insert(n2);

  ASSERT_TRUE(tree.verify_integrity());

  tree.remove(n2);

  ASSERT_TRUE(tree.verify_integrity());

  tree.remove(n1);

  ASSERT_TRUE(tree.verify_integrity());
}

TEST(RBTreeTest, LinearInsertionLinearDeletionTest) {
  auto tree = RBTree<Node, NodeTraits>();

  Node nodes[TESTSIZE];

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    nodes[i] = Node(i);

    tree.insert(nodes[i]);
  }

  ASSERT_TRUE(tree.verify_integrity());

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
/*
    std::string fname = std::string("/tmp/trees/before-") + std::to_string(i) + std::string(".dot");
    tree.dump_to_dot(fname);*/

    //std::cout << "\n\n Removing " << i << "\n";
    tree.remove(nodes[i]);

    /*fname = std::string("/tmp/trees/after-") + std::to_string(i) + std::string(".dot");
    tree.dump_to_dot(fname);*/

    ASSERT_TRUE(tree.verify_integrity());
  }
}

TEST(RBTreeTest, LinearInsertionRandomDeletionTest) {
  auto tree = RBTree<Node, NodeTraits>();

  Node nodes[TESTSIZE];
  std::vector<unsigned int> indices;

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    nodes[i] = Node(i);

    tree.insert(nodes[i]);
    indices.push_back(i);
  }

  std::mt19937 rng(4); // chosen by fair xkcd
  std::random_shuffle(indices.begin(), indices.end(), [&](int i) {
    std::uniform_int_distribution<unsigned int> uni(0,
                                           i - 1);
    return uni(rng);
  });

  ASSERT_TRUE(tree.verify_integrity());

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    //std::string fname = std::string("/tmp/trees/before-") + std::to_string(i) + std::string(".dot");
    //tree.dump_to_dot(fname);

    tree.remove(nodes[indices[i]]);

    //fname = std::string("/tmp/trees/after-") + std::to_string(i) + std::string(".dot");
    //tree.dump_to_dot(fname);

    ASSERT_TRUE(tree.verify_integrity());
  }
}

TEST(RBTreeTest, LinearIterationTest) {
  auto tree = RBTree<Node, NodeTraits>();

  Node nodes[TESTSIZE];

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    nodes[i] = Node(i);
    tree.insert(nodes[i]);
  }

  ASSERT_TRUE(tree.verify_integrity());

  unsigned int i = 0;
  for (auto & n : tree) {
    ASSERT_EQ(n.data, i);
    i++;
  }
}

TEST(RBTreeTest, FindTest) {
  auto tree = RBTree<Node, NodeTraits>();

  Node nodes[TESTSIZE];

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    nodes[i] = Node(2*i);
    tree.insert(nodes[i]);
  }

  // Nonexisting
  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    Node findme(2*i+1);
    auto it = tree.find(findme);
    ASSERT_EQ(it, tree.cend());
  }

  // Existing
  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    Node findme(2*i);
    auto it = tree.find(findme);
    ASSERT_EQ(&(*it), &(nodes[i]));
  }
}

// TODO test equal elements

#endif // TEST_RBTREE_HPP
