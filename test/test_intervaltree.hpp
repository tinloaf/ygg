#ifndef TEST_INTERVALTREE_HPP
#define TEST_INTERVALTREE_HPP

#include "../src/intervaltree.hpp"

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



#endif // TEST_INTERVALTREE_HPP
