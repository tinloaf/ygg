This is Ygg (short for [Yggdrasil](https://en.wikipedia.org/wiki/Yggdrasil)), a C++11 implementation of a Red-Black-Tree as well as an Interval Tree based on the Red-Black-Tree.

If you need a Red-Black- or Interval Tree in your C++ application, and for some reason the existing implementations (like std::set or boost::instrusive::rbtree) are not suited for you, Ygg may be the answer. See the list of features below for why it's awesome!

Features
========

* It's intrusive! Like the containers in boost::intrusive, Ygg follows a 'bring your own data structure' approach. Depending on your use case, this can save a lot of time by avoiding memory allocation and copying stuff around.
* You can be notified when stuff happens. Want to do something to the tree nodes every time a tree rotation happens? Need to know whenever two nodes swap places in the tree? Look no further. Ygg will call methods specified by you on several occasions. In fact, that's how the Interval Tree (the augmented tree version from Cormen et al.) is implemented.
* It's pretty fast. Doing benchmarks correctly is pretty difficult. However, performance should not be too far away from boost::intrusive::rbtree. Comparing against std::set is unfair - std::set loses because it needs to do memory allocation.

Documentation
=============

There's a short usage example below which is probably enough if you just want to use the Red-Black-Tree. More documentation is being added over time here TODO.

Usage Example
=============

This creates a Node class for you (which just holds an int-to-std::string mapping, pretty boring) and sets up an Red-Black-Tree on top of it:

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

Now, let's add some elements, iterate and query:

~~~~~~~~~~~~~{.cpp}
// We need this s.t. we can query by key value (i.e, an int) directly
bool operator<(const Node & lhs, int rhs) {
  return lhs.key < rhs;
}
bool operator<(int lhs, const Node & rhs) {
  return lhs < rhs.key;
}

int main(int argc, char **argv) {
  MyTree t;

  // Storage for the actual nodes.
  // WARNING: using STL containers here can backfire badly. See documentation for details.
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

  // What was the string for i = 3 again?
  auto it = t.find(3); // Note we're using a int to query here, not a Node
  assert(it != t.end());
  std::string retrieved_value = it->value; // *it is the Node

  // Okay, we don't need that Node anymore.
  t.remove(*it);

  // Iterate all the nodes!
  for (const auto & n : t) {
    std::cout << "A node: " << n.value << "\n";
  }
}
~~~~~~~~~~~~~
