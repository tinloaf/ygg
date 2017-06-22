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

Again, a simple example (based on the example above), where we just add a couple of nodes. Note that in this example, we already use the feature that we can query for anything that is comparable to a Node (in this case, we make ints comparable to Nodes first). See TODO for details.

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

  // What was the string for i = 3 again?
  auto it = t.find(3); // Note we're using a int to query here, not a Node
  assert(it != t.end());
  std::string retrieved_value = it->value; // *it is the Node

  // Okay, we don't need that Node anymore.
  t.remove(*it);
}
~~~~~~~~~~~~~

Using ygg::IntervalTree
=================

Using the IntervalTree is similar to using the RBTree. You need to implement your own node. However, Intervals can not just be compared to each other using `operator<(…)`, but the IntervalTree has to have access to the actual upper and lower bounds of the intervals. This is achieved by implementing the `get_upper()` and `get_lower()` methods, and setting a `key_type` typedef (or using) in the NodeTraits. You have to implement NodeTraits yourself for the IntervalTree (as opposed to the Red-Black-Tree). However, these are the only three things you have to implement. See TODO for details on implementing NodeTraits.

Again, an example demonstrates setting up an IntervalTree, this time mapping intervals to strings:

~~~~~~~~~~~~~{.cpp}
#include "ygg.hpp"

using namespace ygg;

template<class Node>
class NodeTraits {
public:
  using key_type = int;

  static int get_lower(const Node & node) {
    return node.lower;
  }

  static int get_upper(const Node & node) {
    return node.upper;
  }
};

// The node class
class Node : public ITreeNodeBase<Node, NodeTraits<Node>> {
public:
  int upper;
  int lower;

  std::string value;

  // No need to implement operator< here. Intervals are compared based on what
  // get_upper() and get_lower() return.
}


// Configure the IntervalTree
using MyTree = IntervalTree<Node, NodeTraits<Node>>;
~~~~~~~~~~~~~

Setting up a tree and inserting intervals is very similar to an RBTree:

~~~~~~~~~~~~~{.cpp}
int main(int argc, char **argv) {
  MyTree t;

  // Storage for the actual nodes.
  // WARNING: using STL containers here can backfire badly. See TODO.
  Node nodes[5];

  // Initialize the nodes with some values
  for (size_t i = 0 ; i < 5 ; ++i) {
    nodes[i].lower = i;
    nodes[i].upper = i + 10;
    nodes[i].value = std::string("The interval is [") + std::to_string(i) + std::string("]");
  }

  // Insert them
  for (size_t i = 0 ; i < 5 ; ++i) {
    t.insert(nodes[i]);
  }

  // Querying explained below

  // Delete one node
  t.remove(nodes[3]);
}
~~~~~~~~~~~~~

Querying is a bit different from the RBTree. Since an IntervalTree is just an RBTree, you can of course again use find() to find an interval that you have previously inserted, but the really interesting queries are overlap queries.

Before explaining them, let me show you how the concept of types 'Comparable' to the nodes works with IntervalTrees. For a type T to be comparable to nodes,  in RBTrees, you needed to implement `operator<`. However, that's not used in IntervalTrees, so what to do? Easy: Your NodeTraits class must provide `get_upper()` and `get_lower()` methods for the comparable type T, too.

Let's say we want to represent intervals by a `std::pair<int, int>`, and we want to make this comparable to our Nodes. We would change the NodeTraits like so:


~~~~~~~~~~~~~{.cpp}
…

using Interval = std::pair<int, int>;

template<class Node>
class NodeTraits {
public:
  using key_type = int;

  static int get_lower(const Node & node) {
    return node.lower;
  }

  static int get_upper(const Node & node) {
    return node.upper;
  }

  static int get_lower(const Interval & i) {
    return std::get<0>(i);
  }

  static int get_upper(const Interval & i) {
    return std::get<1>(i);
  }
};

…
~~~~~~~~~~~~~

Now you can query for nodes overlapping a certain interval:

~~~~~~~~~~~~~{.cpp}
…

int main(int argc, char ** argv) {
  …

  Interval query_range {2, 3};

  for (const auto & node : t.query(query_range))
  {
    // This will be called with nodes 2 and 3
  }

  …
}
~~~~~~~~~~~~~
