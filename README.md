[![Build Status](https://travis-ci.org/tinloaf/ygg.svg?branch=master)](https://travis-ci.org/tinloaf/ygg)

This is Ygg (short for [Yggdrasil](https://en.wikipedia.org/wiki/Yggdrasil)), a C++14
implementation of several intrusive data structures:

* a Red-Black Tree
* a Zip Tree
* an Interval Tree
* an Interval Map
* a Doubly-Linked List
* a Dynamic Segment Tree (which is something between a segment tree and an interval map)


If you need a Red-Black-Tree, a Zip Tree, an Interval Tree or an Interval Map in your C++ application, and for 
some reason the existing implementations (like std::set or boost::instrusive::rbtree) are not 
suited for you, Ygg may be the answer. Also, I do not know of any other implementation of what I 
call the "Dynamic Segment Tree" (if you know something similar, please let me know!)

See the list of features below for why Ygg is awesome!

If you're not sure whether one of these data structures is the right data structure for your 
application, check out the datastructure overview in the 
[Documentation](https://tinloaf.github.io/ygg/).

**Warning**: This is still in development. Therefore, things could break. Also, the API
might change. If you found an example where one of the data structures does not behave as 
expected, please let me know.

Features
========

* It's intrusive! Like the containers in boost::intrusive, Ygg follows a 'bring your own data structure' approach. Depending on your use case, this can save a lot of time by avoiding memory allocation and copying stuff around.
* You can be notified when stuff happens. Want to do something to the tree nodes every time a tree rotation happens? Need to know whenever two nodes swap places in the tree? Look no further. Ygg will call methods specified by you on several occasions. In fact, that's how the Interval Tree (the augmented tree version from Cormen et al.) is implemented.
* It's pretty fast. Doing benchmarks correctly is pretty difficult. However, performance should not be too far away from boost::intrusive::rbtree. Comparing against std::set is unfair - std::set loses because it needs to do memory allocation.

Installation
============

It's a header-only library. (Yes I know, there are .cpp files. I like to keep declaration and definition separated, even if everything's actually a header.) Just make sure everything in the src folder is in your include path, and you're set.

Documentation
=============

There's a short usage example below which is probably enough if you just want to use the 
Red-Black-Tree. 

The [Documentation](https://tinloaf.github.io/ygg/) contains an overview over how the different 
datastructures behave as well as more in-depth examples as well as an API documentation.

Usage Example
=============

This creates a Node class for you (which just holds an int-to-std::string mapping, pretty boring) and sets up an Red-Black-Tree on top of it:

```cpp
#include "ygg.hpp"

using namespace ygg;

// The tree options
using MyTreeOptions = TreeOptions<TreeFlags::MULTIPLE>;

// The node class
class Node : public RBTreeNodeBase<Node, MyTreeOptions> {
public:
  int key;
  std::string value;

  // need to implement this s.t. we can use the default std::less as comparator
  bool operator<(const Node & other) const {
    return this->key < other.key;
  }
}

// Configure the RBTree based on Node and the default NodeTraits
using MyTree = RBTree<Node, RBDefaultNodeTraits<Node>, MyTreeOptions>;
```

Now, let's add some elements, iterate and query:

```cpp
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
```

License
=======

This software is licensed under the MIT license. See LICENSE.txt for details.
