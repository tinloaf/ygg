This page will guide you through the minimum steps necessary to use ygg::RBTree and ygg::IntervalTree.

Using ygg::RBTree
=================

Setting up your own RBTree basically consists of TODO or TODO steps:

1. Define your Node class, inheriting from RBTreeNodeBase
2. Optionally define your NodeTraits class (see below)
3. Optionally define an own comparator class
4. Create an RBTree based on the Node and NodeTraits class

This example (which uses the defaults for steps 2 and 3 above) demonstrates it, assuming that you want to store a simple integer-to-string map in the tree:

~~~~~~~~~~~~~{.cpp}
#include "ygg.hpp"

using namespace ygg;

// The node class
class Node : public RBTreeNodeBase<Node> {
public:
  int key;
  std::string value;

  // need to implement this s.t. we can use the default std::less as comparator
  bool operator<(const Node & other) const {
    return this->key < other.key;
  }
}

// Configure the RBTree based on Node and the default NodeTraits
using MyTree = RBTree<Node, RBDefaultNodeTraits<Node>>;
~~~~~~~~~~~~~


Inserting values into the tree is then a TODO steps process:

1. Allocate memory for the new node (you should probably allocate multiple in one go?)
2. Set the node data
3. Insert the node into the tree

Again, a simple example (based on the example above), where we just add a couple of nodes:

~~~~~~~~~~~~~{.cpp}
int main(int argc, char **argv) {
  MyTree t;

  // Storage for the actual nodes.
  // WARNING: using STL containers here can backfire badly. See TODO.
  Node nodes[5];

  // Initialize the nodes with some values
  for (size_t i = 0 ; i < 5 ; ++i) {
    nodes[i].key = i;
    nodes[i].value = std::string("The key is ") + std::to_string(i);
  }

  // Insert them
  for (size_t i = 0 ; i < 5 ; ++i) {
    t.insert(nodes[i]);
  }
}
~~~~~~~~~~~~~
