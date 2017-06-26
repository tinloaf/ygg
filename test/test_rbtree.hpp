#ifndef TEST_RBTREE_HPP
#define TEST_RBTREE_HPP

#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <algorithm>

#include "../src/rbtree.hpp"

using namespace ygg;

#define TESTSIZE 2000

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

class EqualityNode : public RBTreeNodeBase<EqualityNode, true> {
public:
  int data;

  EqualityNode () : data(0) {};
  explicit EqualityNode(int data_in) : data(data_in) {};
  EqualityNode(const EqualityNode &other) : data(other.data) {};

  bool operator<(const EqualityNode & other) const {
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

class EqualityNodeTraits {
public:
  static std::string get_id(const EqualityNode * node) {
    return std::to_string(node->data);
  }

  static void leaf_inserted(EqualityNode & node) { (void)node ; };
  static void rotated_left(EqualityNode & node) { (void)node ; };
  static void rotated_right(EqualityNode & node) { (void)node ; };
  static void deleted_below(EqualityNode & node) { (void)node ; };
  static void swapped(EqualityNode & n1, EqualityNode & n2) { (void)n1 ; (void)n2 ; };
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

  std::set<int> values_seen;
  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    int val = uni(rng);
    while (values_seen.find(val) != values_seen.end()) {
      val = uni(rng);
    }
    nodes[i] = Node(val);
    values_seen.insert(val);

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

TEST(RBTreeTest, LinearMultipleIterationTest) {
  auto tree = RBTree<EqualityNode, EqualityNodeTraits, true>();

  EqualityNode nodes[TESTSIZE*5];

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    for (unsigned j = 0 ; j < 5 ; ++j) {
      nodes[5*i + j] = EqualityNode(i);
      tree.insert(nodes[5*i + j]);
    }
  }

  ASSERT_TRUE(tree.verify_integrity());

  unsigned int i = 0;
  for (auto & n : tree) {
    ASSERT_EQ(n.data, nodes[i].data);
    i++;
  }
}

TEST(RBTreeTest, LinearIterationTest) {
  auto tree = RBTree<Node, NodeTraits>();

  Node nodes[TESTSIZE];
  std::vector<size_t> indices;
  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    nodes[i] = Node(i);
    indices.push_back(i);
  }

  std::mt19937 rng(4); // chosen by fair xkcd
  std::random_shuffle(indices.begin(), indices.end(), [&](int i) {
    std::uniform_int_distribution<unsigned int> uni(0,
                                           i - 1);
    return uni(rng);
  });

  for (auto index : indices) {
    tree.insert(nodes[index]);
  }

  ASSERT_TRUE(tree.verify_integrity());

  unsigned int i = 0;
  for (auto & n : tree) {
    ASSERT_EQ(n.data, i);
    i++;
  }
}

TEST(RBTreeTest, ReverseIterationTest) {
  auto tree = RBTree<Node, NodeTraits>();

  Node nodes[TESTSIZE];
  std::vector<size_t> indices;
  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    nodes[i] = Node(i);
    indices.push_back(i);
  }

  std::mt19937 rng(4); // chosen by fair xkcd
  std::random_shuffle(indices.begin(), indices.end(), [&](int i) {
    std::uniform_int_distribution<unsigned int> uni(0,
                                           i - 1);
    return uni(rng);
  });

  for (auto index : indices) {
    tree.insert(nodes[index]);
  }

  ASSERT_TRUE(tree.verify_integrity());

  auto it = tree.rbegin();
  unsigned int i = TESTSIZE - 1;
  while (it != tree.rend()) {
    ASSERT_EQ(it->data, i);
    it++;
    i--;
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

TEST(RBTreeTest, ComprehensiveTest) {
  auto tree = RBTree<Node, NodeTraits>();

  Node persistent_nodes[TESTSIZE];
  std::vector<unsigned int> indices;
  std::mt19937 rng(4); // chosen by fair xkcd

  std::set<unsigned int> values_seen;

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    unsigned int data = 10 * i;
    persistent_nodes[i] = Node(data);
    indices.push_back(i);
    values_seen.insert(data);
  }

  std::random_shuffle(indices.begin(), indices.end(), [&](int i) {
    std::uniform_int_distribution<unsigned int> uni(0,
                                           i - 1);
    return uni(rng);
  });

  for (auto index : indices) {
    tree.insert(persistent_nodes[index]);
  }

  ASSERT_TRUE(tree.verify_integrity());

  Node transient_nodes[TESTSIZE];
  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    std::uniform_int_distribution<unsigned int> uni(0, 10 * (TESTSIZE + 1));
    unsigned int data = uni(rng);

    while (values_seen.find(data) != values_seen.end()) {
      data = uni(rng);
    }

    transient_nodes[i] = Node(data);

    values_seen.insert(data);

    //std::cout << "Inserting random value: " << data << "\n";
  }

  std::random_shuffle(indices.begin(), indices.end(), [&](int i) {
    std::uniform_int_distribution<unsigned int> uni(0,
                                           i - 1);
    return uni(rng);
  });

  for (auto index : indices) {
    tree.insert(transient_nodes[index]);
  }

  ASSERT_TRUE(tree.verify_integrity());

  //std::string fname_before = std::string("/tmp/trees/rbt-comprehensive-before.dot");
  //tree.dump_to_dot(fname_before);

  for (int i = 0 ; i < TESTSIZE ; ++i) {
    tree.remove(transient_nodes[i]);


    //std::string rem_fname = std::string("/tmp/trees/removed-") + std::to_string(i) + std::string(".dot");
    //std::cout << "Step " << i << ": removing data " << transient_nodes[i].data << "\n";
    //tree.dump_to_dot(rem_fname);
    ASSERT_TRUE(tree.verify_integrity());
  }

  //std::string fname = std::string("/tmp/trees/rbt-comprehensive.dot");
  //tree.dump_to_dot(fname);

  // Query elements
  for (int i = 0 ; i < TESTSIZE ; ++i) {
    auto it = tree.find(persistent_nodes[i]);
    assert(&(*it) == &(persistent_nodes[i]));
    ASSERT_EQ(&(*it), &(persistent_nodes[i]));
  }

}


TEST(RBTreeTest, ComprehensiveMultipleTest) {
  auto tree = RBTree<EqualityNode, EqualityNodeTraits, true>();

  EqualityNode persistent_nodes[TESTSIZE];
  std::vector<unsigned int> indices;
  std::mt19937 rng(4); // chosen by fair xkcd

  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    unsigned int data = 10 * i;
    persistent_nodes[i] = EqualityNode(data);
    indices.push_back(i);
  }

  std::random_shuffle(indices.begin(), indices.end(), [&](int i) {
    std::uniform_int_distribution<unsigned int> uni(0,
                                           i - 1);
    return uni(rng);
  });

  for (auto index : indices) {
    tree.insert(persistent_nodes[index]);
  }

  ASSERT_TRUE(tree.verify_integrity());

  EqualityNode transient_nodes[TESTSIZE];
  for (unsigned int i = 0 ; i < TESTSIZE ; ++i) {
    std::uniform_int_distribution<unsigned int> uni(0, 10 * (TESTSIZE + 1));
    unsigned int data = uni(rng);

    transient_nodes[i] = EqualityNode(data);
    //std::cout << "Inserting random value: " << data << "\n";
  }

  std::random_shuffle(indices.begin(), indices.end(), [&](int i) {
    std::uniform_int_distribution<unsigned int> uni(0,
                                           i - 1);
    return uni(rng);
  });

  for (auto index : indices) {
    tree.insert(transient_nodes[index]);
  }

  ASSERT_TRUE(tree.verify_integrity());

  //std::string fname_before = std::string("/tmp/trees/rbt-comprehensive-before.dot");
  //tree.dump_to_dot(fname_before);

  for (int i = 0 ; i < TESTSIZE ; ++i) {
    tree.remove(transient_nodes[i]);

    //std::string rem_fname = std::string("/tmp/trees/removed-") + std::to_string(i) + std::string(".dot");
    //std::cout << "Step " << i << ": removing data " << transient_nodes[i].data << "\n";
    //tree.dump_to_dot(rem_fname);
    ASSERT_TRUE(tree.verify_integrity());
  }


  //std::string fname = std::string("/tmp/trees/rbt-comprehensive.dot");
  //tree.dump_to_dot(fname);

  // Query elements
  for (int i = 0 ; i < TESTSIZE ; ++i) {
    //std::cout << "Finding " << i << "\n";
    auto it = tree.find(persistent_nodes[i]);
    assert(&(*it) == &(persistent_nodes[i]));
    ASSERT_EQ(&(*it), &(persistent_nodes[i]));
  }

}
// TODO test equal elements

#endif // TEST_RBTREE_HPP
