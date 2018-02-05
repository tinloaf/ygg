#ifndef TEST_RBTREE_HPP
#define TEST_RBTREE_HPP

#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <algorithm>

#include "../src/rbtree.hpp"

using namespace ygg;

#define RBTREE_TESTSIZE 2000

class Node : public RBTreeNodeBase<Node, TreeOptions<>> { // No multi-nodes!
public:
  int data;

  Node () : data(0) {};
  explicit Node(int data_in) : data(data_in) {};
  Node(const Node &other) : data(other.data) {};

  bool operator<(const Node & other) const {
    return this->data < other.data;
  }
};

class EqualityNode : public RBTreeNodeBase<EqualityNode> {
public:
  int data;
  int sub_data;

  EqualityNode () : data(0) {};
  explicit EqualityNode(int data_in, int sub_data_in = 0) : data(data_in), sub_data(sub_data_in) {};
  EqualityNode(const EqualityNode &other) : data(other.data), sub_data(other.sub_data) {};

  bool operator<(const EqualityNode & other) const {
    return this->data < other.data;
  }
};

class NodeTraits : public RBDefaultNodeTraits<Node> {
public:
  static std::string get_id(const Node * node) {
    return std::to_string(node->data);
  }
};

class EqualityNodeTraits : public RBDefaultNodeTraits<EqualityNode> {
public:
  static std::string get_id(const EqualityNode * node) {
    return std::string("(") + std::to_string(node->data) + std::string("/") + std::to_string(node->sub_data) + std::string(")");
  }
};

TEST(RBTreeTest, TrivialInsertionTest) {
  auto tree = RBTree<Node, NodeTraits, TreeOptions<>>();

  Node n;
  n.data = 0;
  tree.insert(n);
	ASSERT_FALSE(tree.empty());
  ASSERT_TRUE(tree.verify_integrity());
}

TEST(RBTreeTest, TrivialSizeTest) {
  auto tree = RBTree<EqualityNode, EqualityNodeTraits>();

  EqualityNode n;
  n.data = 0;

  ASSERT_EQ(tree.size(), 0);
  ASSERT_TRUE(tree.empty());

  tree.insert(n);

  ASSERT_EQ(tree.size(), 1);
  ASSERT_FALSE(tree.empty());

	tree.clear();
	ASSERT_EQ(tree.size(), 0);
	ASSERT_TRUE(tree.empty());
}

TEST(RBTreeTest, RandomInsertionTest) {
  auto tree = RBTree<Node, NodeTraits, TreeOptions<>>();

  std::mt19937 rng(4); // chosen by fair xkcd
  std::uniform_int_distribution<int> uni(std::numeric_limits<int>::min(),
                                         std::numeric_limits<int>::max());

  Node nodes[RBTREE_TESTSIZE];

  std::set<int> values_seen;
  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
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
  auto tree = RBTree<Node, NodeTraits, TreeOptions<>>();

  Node nodes[RBTREE_TESTSIZE];

  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    nodes[i] = Node(i);

    tree.insert(nodes[i]);

    //std::string fname = std::string("/tmp/trees/tree-") + std::to_string(i) + std::string(".dot");
    //tree.dump_to_dot(fname);

    ASSERT_TRUE(tree.verify_integrity());
  }
}

TEST(RBTreeTest, HintedPostEqualInsertionTest) {
  auto tree = RBTree<EqualityNode, EqualityNodeTraits>();

  EqualityNode n_insert_before(1, 0);
  EqualityNode n_pre(1, 1);
  EqualityNode n_insert_between(1, 2);
  EqualityNode n_post(2, 3);

  tree.insert(n_pre);
  tree.insert(n_post);

  ASSERT_TRUE(tree.verify_integrity());

  // should be inserted before pre
  tree.insert(n_insert_before, n_pre);

  // should be inserted between pre and post
  tree.insert(n_insert_between, n_post);


  auto it = tree.begin();
  ASSERT_EQ(it->sub_data, 0);
  it++;
  ASSERT_EQ(it->sub_data, 1);
  it++;
  ASSERT_EQ(it->sub_data, 2);
  it++;
  ASSERT_EQ(it->sub_data, 3);
  it++;
  ASSERT_EQ(it, tree.end());
}

TEST(RBTreeTest, RepeatedHintedPostEqualInsertionTest) {
  auto tree = RBTree<EqualityNode, EqualityNodeTraits>();

  EqualityNode nodes_pre[RBTREE_TESTSIZE];
  EqualityNode nodes_post[RBTREE_TESTSIZE];
  EqualityNode nodes_between[RBTREE_TESTSIZE];
  EqualityNode node_border_small(1, RBTREE_TESTSIZE + 2);
  EqualityNode node_border_large(2, RBTREE_TESTSIZE + 2);

  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    nodes_pre[i] = EqualityNode(1, i);
    nodes_post[i] = EqualityNode(2, i);
    nodes_between[i] = EqualityNode(1, RBTREE_TESTSIZE + 1);
  }

  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    tree.insert(nodes_post[i], tree.end()); // insert in order
  }

  tree.insert(nodes_pre[RBTREE_TESTSIZE-1], nodes_post[0]);

  for (int i = RBTREE_TESTSIZE - 2 ; i >= 0 ; --i) {
    tree.insert(nodes_pre[i], nodes_pre[i+1]);
    ASSERT_EQ(tree.begin()->sub_data, i);
  }

  for (int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    tree.insert(nodes_between[i], nodes_pre[i]);
  }

  tree.insert(node_border_large, nodes_post[0]);
  tree.insert(node_border_small, node_border_large);
  ASSERT_TRUE(tree.verify_integrity());

  auto it = tree.begin();
  for (int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    ASSERT_EQ(it->data, 1);
    ASSERT_EQ(it->sub_data, RBTREE_TESTSIZE + 1); // first, the 'between' node
    it++;
    ASSERT_EQ(it->data, 1);
    ASSERT_EQ(it->sub_data, i); // now, the pre-node
    it++;
  }

  ASSERT_EQ(it->data, 1);
  ASSERT_EQ(it->sub_data, RBTREE_TESTSIZE + 2); // small border
  it++;
  ASSERT_EQ(it->data, 2);
  ASSERT_EQ(it->sub_data, RBTREE_TESTSIZE + 2); // large border
  it++;

  for (int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    ASSERT_EQ(it->data, 2);
    ASSERT_EQ(it->sub_data, i); // post-nodes
    it++;
  }
}

TEST(RBTreeTest, LinearEndHintedInsertionTest) {
  auto tree = RBTree<Node, NodeTraits, TreeOptions<>>();

  Node nodes[RBTREE_TESTSIZE];

  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    nodes[i] = Node(i);
  }

  tree.insert(nodes[RBTREE_TESTSIZE - 1]);

  for (int i = RBTREE_TESTSIZE - 2 ; i >= 0 ; --i) {
    tree.insert(nodes[i], nodes[RBTREE_TESTSIZE - 1]);
    ASSERT_TRUE(tree.verify_integrity());
  }

  int i = 0;
  for (const auto & n : tree) {
    ASSERT_EQ(n.data, i);
    i++;
  }
}

TEST(RBTreeTest, HinterOrderPreservationTest) {
  auto tree = RBTree<EqualityNode, EqualityNodeTraits>();

  EqualityNode nodes[3*RBTREE_TESTSIZE];

  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    nodes[3*i] = EqualityNode(i, 0);
    nodes[3*i + 1] = EqualityNode(i, 1);
    nodes[3*i + 2] = EqualityNode(i, 2);
  }

  // insert the middles
  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    tree.insert(nodes[3*i + 1]);
  }

  tree.verify_integrity();

  // insert the prefix, using a hint
  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    tree.insert(nodes[3*i], nodes[3*i + 1]);
  }

  tree.verify_integrity();

  // insert the postfix, using a hint
  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE - 1 ; ++i) {
    tree.insert(nodes[3*i + 2], nodes[3*i + 3]);
  }

  unsigned int i = 0;
  for (auto & n : tree) {
    ASSERT_EQ(n.data, i / 3);
    ASSERT_EQ(n.sub_data, i % 3);
    ++i;
  }
}

TEST(RBTreeTest, LinearNextHintedInsertionTest) {
  auto tree = RBTree<Node, NodeTraits, TreeOptions<>>();

  Node nodes[RBTREE_TESTSIZE];

  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    nodes[i] = Node(i);
  }

  tree.insert(nodes[RBTREE_TESTSIZE - 1]);

  for (int i = RBTREE_TESTSIZE - 2 ; i >= 0 ; --i) {
    tree.insert(nodes[i], nodes[i+1]);
    ASSERT_TRUE(tree.verify_integrity());
  }

  int i = 0;
  for (const auto & n : tree) {
    ASSERT_EQ(n.data, i);
    i++;
  }
}

TEST(RBTreeTest, LowerBoundTest) {
  auto tree = RBTree<Node, NodeTraits, TreeOptions<>>();

  Node nodes[RBTREE_TESTSIZE];

  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    nodes[i] = Node(2*i);
    tree.insert(nodes[i]);
  }

  ASSERT_TRUE(tree.verify_integrity());

  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE - 1 ; ++i) {
    Node query_next(2*i + 1);
    auto it_next = tree.lower_bound(query_next);
    ASSERT_EQ(it_next->data, nodes[i+1].data);

    Node query(2*i);
    auto it = tree.lower_bound(query);
    // We look for "not less"
    ASSERT_EQ(it->data, nodes[i].data);
  }

  Node query(2*(RBTREE_TESTSIZE - 1) + 1);
  auto it = tree.lower_bound(query);
  ASSERT_EQ(it, tree.end());
}

TEST(RBTreeTest, UpperBoundTest) {
  auto tree = RBTree<Node, NodeTraits, TreeOptions<>>();

  Node nodes[RBTREE_TESTSIZE];

  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    nodes[i] = Node(2*i);
    tree.insert(nodes[i]);
  }

  ASSERT_TRUE(tree.verify_integrity());

  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE - 1 ; ++i) {
    Node query_next(2*i + 1);
    auto it_next = tree.upper_bound(query_next);
    ASSERT_EQ(it_next->data, nodes[i+1].data);

    Node query(2*i);
    auto it = tree.upper_bound(query);
    // We look for "truly greater"
    ASSERT_EQ(it->data, nodes[i+1].data);
  }

  Node query(2*(RBTREE_TESTSIZE - 1) + 1);
  auto it = tree.upper_bound(query);
  ASSERT_EQ(it, tree.end());
}

TEST(RBTreeTest, TrivialDeletionTest) {
  auto tree = RBTree<Node, NodeTraits, TreeOptions<>>();

  Node n1;
  n1.data = 0;
  tree.insert(n1);

  Node n2;
  n2.data = 1;
  tree.insert(n2);

  ASSERT_FALSE(tree.empty());
  ASSERT_TRUE(tree.verify_integrity());

  tree.remove(n2);

  ASSERT_TRUE(tree.verify_integrity());

  tree.remove(n1);

  ASSERT_TRUE(tree.verify_integrity());
  ASSERT_TRUE(tree.empty());
}

TEST(RBTreeTest, LinearInsertionLinearDeletionTest) {
  auto tree = RBTree<Node, NodeTraits, TreeOptions<>>();

  Node nodes[RBTREE_TESTSIZE];

  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    nodes[i] = Node(i);

    tree.insert(nodes[i]);
  }

  ASSERT_TRUE(tree.verify_integrity());

  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    //std::cout << "\n\n Removing " << i << "\n";
    tree.remove(nodes[i]);

    ASSERT_TRUE(tree.verify_integrity());
  }
}

TEST(RBTreeTest, LinearInsertionRandomDeletionTest) {
  auto tree = RBTree<Node, NodeTraits, TreeOptions<>>();

  Node nodes[RBTREE_TESTSIZE];
  std::vector<unsigned int> indices;

  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
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

  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    //std::string fname = std::string("/tmp/trees/before-") + std::to_string(i) + std::string(".dot");
    //tree.dump_to_dot(fname);

    tree.remove(nodes[indices[i]]);

    //fname = std::string("/tmp/trees/after-") + std::to_string(i) + std::string(".dot");
    //tree.dump_to_dot(fname);

    ASSERT_TRUE(tree.verify_integrity());
  }
}

TEST(RBTreeTest, LinearMultipleIterationTest) {
  auto tree = RBTree<EqualityNode, EqualityNodeTraits>();

  EqualityNode nodes[RBTREE_TESTSIZE*5];

  std::vector<size_t> indices;

  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    for (unsigned j = 0 ; j < 5 ; ++j) {
      nodes[5*i + j] = EqualityNode(i);
      indices.push_back(5*i + j);
    }
  }

  std::mt19937 rng(4); // chosen by fair xkcd
  std::random_shuffle(indices.begin(), indices.end(), [&](int i) {
    std::uniform_int_distribution<unsigned int> uni(0, i - 1);
    return uni(rng);
  });

  size_t size = 0;
  for (auto index : indices) {
    tree.insert(nodes[index]);
    size++;
    ASSERT_EQ(tree.size(), size);
  }

  ASSERT_TRUE(tree.verify_integrity());

  unsigned int i = 0;
  for (auto & n : tree) {
    ASSERT_EQ(n.data, nodes[i].data);
    i++;
  }
}

TEST(RBTreeTest, LinearIterationTest) {
  auto tree = RBTree<Node, NodeTraits, TreeOptions<>>();

  Node nodes[RBTREE_TESTSIZE];
  std::vector<size_t> indices;
  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    nodes[i] = Node(i);
    indices.push_back(i);
  }

  std::mt19937 rng(4); // chosen by fair xkcd
  std::random_shuffle(indices.begin(), indices.end(), [&](int i) {
    std::uniform_int_distribution<unsigned int> uni(0, i - 1);
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
  auto tree = RBTree<Node, NodeTraits, TreeOptions<>>();

  Node nodes[RBTREE_TESTSIZE];
  std::vector<size_t> indices;
  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
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
  unsigned int i = RBTREE_TESTSIZE - 1;
  while (it != tree.rend()) {
    ASSERT_EQ(it->data, i);
    it++;
    i--;
  }
}

TEST(RBTreeTest, FindTest) {
  auto tree = RBTree<Node, NodeTraits, TreeOptions<>>();

  Node nodes[RBTREE_TESTSIZE];

  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    nodes[i] = Node(2*i);
    tree.insert(nodes[i]);
  }

  // Nonexisting
  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    Node findme(2*i+1);
    auto it = tree.find(findme);
    ASSERT_EQ(it, tree.end());
  }

  // Existing
  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    Node findme(2*i);
    auto it = tree.find(findme);
    ASSERT_EQ(&(*it), &(nodes[i]));
  }
}

TEST(RBTreeTest, ComprehensiveTest) {
  auto tree = RBTree<Node, NodeTraits, TreeOptions<>>();

  Node persistent_nodes[RBTREE_TESTSIZE];
  std::vector<unsigned int> indices;
  std::mt19937 rng(4); // chosen by fair xkcd

  std::set<unsigned int> values_seen;

  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
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

  Node transient_nodes[RBTREE_TESTSIZE];
  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    std::uniform_int_distribution<unsigned int> uni(0, 10 * (RBTREE_TESTSIZE + 1));
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

  for (int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    tree.remove(transient_nodes[i]);


    //std::string rem_fname = std::string("/tmp/trees/removed-") + std::to_string(i) + std::string(".dot");
    //std::cout << "Step " << i << ": removing data " << transient_nodes[i].data << "\n";
    //tree.dump_to_dot(rem_fname);
    ASSERT_TRUE(tree.verify_integrity());
  }

  //std::string fname = std::string("/tmp/trees/rbt-comprehensive.dot");
  //tree.dump_to_dot(fname);

  // Query elements
  for (int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    auto it = tree.find(persistent_nodes[i]);
    assert(&(*it) == &(persistent_nodes[i]));
    ASSERT_EQ(&(*it), &(persistent_nodes[i]));
  }

}


TEST(RBTreeTest, ComprehensiveMultipleTest) {
  auto tree = RBTree<EqualityNode, EqualityNodeTraits>();

  EqualityNode persistent_nodes[RBTREE_TESTSIZE];
  std::vector<unsigned int> indices;
  std::mt19937 rng(4); // chosen by fair xkcd

  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    unsigned int data = 10 * i;
    persistent_nodes[i] = EqualityNode(data);
    indices.push_back(i);
  }

  std::random_shuffle(indices.begin(), indices.end(), [&](int i) {
    std::uniform_int_distribution<unsigned int> uni(0,
                                           i - 1);
    return uni(rng);
  });

  size_t size = 0;
  for (auto index : indices) {
    tree.insert(persistent_nodes[index]);
    size++;
  }

  ASSERT_TRUE(tree.verify_integrity());

  EqualityNode transient_nodes[RBTREE_TESTSIZE];
  for (unsigned int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    std::uniform_int_distribution<unsigned int> uni(0, 10 * (RBTREE_TESTSIZE + 1));
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
    size++;
  }

  ASSERT_EQ(tree.size(), size);
  ASSERT_TRUE(tree.verify_integrity());

  //std::string fname_before = std::string("/tmp/trees/rbt-comprehensive-before.dot");
  //tree.dump_to_dot(fname_before);

  for (int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    tree.remove(transient_nodes[i]);
    size--;
    //std::string rem_fname = std::string("/tmp/trees/removed-") + std::to_string(i) + std::string(".dot");
    //std::cout << "Step " << i << ": removing data " << transient_nodes[i].data << "\n";
    //tree.dump_to_dot(rem_fname);
    ASSERT_TRUE(tree.verify_integrity());
    ASSERT_EQ(tree.size(), size);
  }


  //std::string fname = std::string("/tmp/trees/rbt-comprehensive.dot");
  //tree.dump_to_dot(fname);

  // Query elements
  for (int i = 0 ; i < RBTREE_TESTSIZE ; ++i) {
    //std::cout << "Finding " << i << "\n";
    auto it = tree.find(persistent_nodes[i]);
    assert(&(*it) == &(persistent_nodes[i]));
    ASSERT_EQ(&(*it), &(persistent_nodes[i]));
  }

}
// TODO test equal elements

#endif // TEST_RBTREE_HPP
