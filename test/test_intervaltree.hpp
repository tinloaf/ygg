#ifndef TEST_INTERVALTREE_HPP
#define TEST_INTERVALTREE_HPP

#include "../src/intervaltree.hpp"

#define IT_TESTSIZE 5000

template<class Node>
class MyNodeTraits {
public:
  using key_type = unsigned int;

  static unsigned int get_lower(const Node & node) {
    return node.lower;
  }


  static unsigned int get_upper(const Node & node) {
    return node.upper;
  }

  static std::string get_id(const Node * node) {
    return std::to_string(node->data);
  }
};

class ITNode : public ITreeNodeBase<ITNode, MyNodeTraits<Node>> {
public:
  int data;
  unsigned int lower;
  unsigned int upper;

  ITNode () : data(0), lower(0), upper(0) {};
  explicit ITNode(unsigned int lower_in, unsigned int upper_in, int data_in) : data(data_in), lower(lower_in), upper(upper_in) {};
  ITNode(const ITNode &other) : data(other.data), lower(other.lower), upper(other.upper) {};
};

TEST(ITreeTest, TrivialInsertionTest) {
  auto tree = IntervalTree<ITNode, MyNodeTraits<ITNode>>();

  ITNode n(0, 10, 0);
  tree.insert(n);

  ASSERT_TRUE(tree.verify_integrity());
}

TEST(ITreeTest, RandomInsertionTest) {
  auto tree = IntervalTree<ITNode, MyNodeTraits<ITNode>>();

  ITNode nodes[IT_TESTSIZE];
  std::mt19937 rng(4); // chosen by fair xkcd

  for (unsigned int i = 0 ; i < IT_TESTSIZE ; ++i) {
    std::uniform_int_distribution<unsigned int> bounds_distr(0,
                      std::numeric_limits<unsigned int>::max() / 2);
    unsigned int lower = bounds_distr(rng);
    unsigned int upper = lower + bounds_distr(rng);

    nodes[i] = ITNode(lower, upper, i);

    //std::string fname = std::string("/tmp/trees/before-") + std::to_string(i) + std::string(".dot");
    //tree.dump_to_dot(fname);

    tree.insert(nodes[i]);

    //fname = std::string("/tmp/trees/after-") + std::to_string(i) + std::string(".dot");
    //tree.dump_to_dot(fname);

    ASSERT_TRUE(tree.verify_integrity());

  }
}

TEST(ITreeTest, RandomInsertionRandomDeletionTest) {
  auto tree = IntervalTree<ITNode, MyNodeTraits<ITNode>>();

  ITNode nodes[IT_TESTSIZE];
  std::vector<unsigned int> indices;
  std::mt19937 rng(4); // chosen by fair xkcd


  for (unsigned int i = 0 ; i < IT_TESTSIZE ; ++i) {
    std::uniform_int_distribution<unsigned int> bounds_distr(0,
                      std::numeric_limits<unsigned int>::max() / 2);
    unsigned int lower = bounds_distr(rng);
    unsigned int upper = lower + bounds_distr(rng);

    nodes[i] = ITNode(lower, upper, i);

    tree.insert(nodes[i]);
    indices.push_back(i);
  }

  std::random_shuffle(indices.begin(), indices.end(), [&](int i) {
    std::uniform_int_distribution<unsigned int> uni(0,
                                           i - 1);
    return uni(rng);
  });

  ASSERT_TRUE(tree.verify_integrity());

  for (unsigned int i = 0 ; i < IT_TESTSIZE ; ++i) {
    std::string fname = std::string("/tmp/trees/before-") + std::to_string(i) + std::string(".dot");
    tree.dump_to_dot(fname);

    tree.remove(nodes[indices[i]]);

    fname = std::string("/tmp/trees/after-") + std::to_string(i) + std::string(".dot");
    tree.dump_to_dot(fname);

    ASSERT_TRUE(tree.verify_integrity());
  }
}



TEST(ITreeTest, TrivialQueryTest) {
  auto tree = IntervalTree<ITNode, MyNodeTraits<ITNode>>();

  ITNode n(10, 20, 0);
  tree.insert(n);

  ASSERT_TRUE(tree.verify_integrity());

  bool found = false;
  ITNode containing(0, 30, 1);
  for (const auto & node : tree.query(containing)) {
    ASSERT_EQ(&n, &node);
    found = true;
  }
  ASSERT_TRUE(found);

  found = false;
  ITNode contained(11, 19, 1);
  for (const auto & node : tree.query(contained)) {
    ASSERT_EQ(&n, &node);
    found = true;
  }
  ASSERT_TRUE(found);

  found = false;
  ITNode overlap_left(0, 15, 1);
  for (const auto & node : tree.query(overlap_left)) {
    ASSERT_EQ(&n, &node);
    found = true;
  }
  ASSERT_TRUE(found);

  found = false;
  ITNode overlap_right(15, 30, 1);
  for (const auto & node : tree.query(overlap_right)) {
    ASSERT_EQ(&n, &node);
    found = true;
  }
  ASSERT_TRUE(found);

  found = false;
  ITNode right(30, 40, 1);
  for (const auto & node : tree.query(right)) {
    (void)node;
    found = true;
  }
  ASSERT_FALSE(found);

  found = false;
  ITNode left(0, 5, 1);
  for (const auto & node : tree.query(left)) {
    (void)node;
    found = true;
  }
  ASSERT_FALSE(found);
}


TEST(ITreeTest, SimpleQueryTest) {
  auto tree = IntervalTree<ITNode, MyNodeTraits<ITNode>>();

  ITNode n1(10, 20, 1);
  tree.insert(n1);
  ITNode n2(21, 30, 2);
  tree.insert(n2);
  ITNode n3(15, 25, 3);
  tree.insert(n3);
  ITNode n4(40, 50, 4);
  tree.insert(n4);


  ASSERT_TRUE(tree.verify_integrity());

  // Should find all
  ITNode query_all(0, 60, 0);
  auto container = tree.query(query_all);
  auto it = container.begin();
  ASSERT_EQ(&(*it), &n1);
  it++;
  ASSERT_EQ(&(*it), &n3);
  it++;
  ASSERT_EQ(&(*it), &n2);
  it++;
  ASSERT_EQ(&(*it), &n4);
  it++;
  ASSERT_EQ(it, container.end());

  // Should find n1, n3
  ITNode query_n1n3(0, 15, 0);
  container = tree.query(query_n1n3);
  it = container.begin();
  ASSERT_EQ(&(*it), &n1);
  it++;
  ASSERT_EQ(&(*it), &n3);
  it++;
  ASSERT_EQ(it, container.end());

  // Should find n3, n2
  ITNode query_n3n2(25, 25, 0);
  container = tree.query(query_n3n2);
  it = container.begin();
  ASSERT_EQ(&(*it), &n3);
  it++;
  ASSERT_EQ(&(*it), &n2);
  it++;
  ASSERT_EQ(it, container.end());

  // Should find n4
  ITNode query_n4(50, 60, 0);
  container = tree.query(query_n4);
  it = container.begin();
  ASSERT_EQ(&(*it), &n4);
  it++;
  ASSERT_EQ(it, container.end());

  // Should find nothing
  ITNode query_left(0, 2, 0);
  container = tree.query(query_left);
  it = container.begin();
  ASSERT_EQ(it, container.end());

  // Should find nothing
  ITNode query_right(70, 80, 0);
  container = tree.query(query_right);
  it = container.begin();
  ASSERT_EQ(it, container.end());
}

#endif // TEST_INTERVALTREE_HPP
