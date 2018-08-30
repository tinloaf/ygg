//
// Created by lukas on 30.08.18.
//

#include "../src/ygg.hpp"

using namespace ygg;

/* The tree options
 *
 * We want ranks to be generated from a node's hash (note that we implement
 * std::hash below). Since std::hash for the int values of our nodes is rather
 * non-universal, we also want universal hashing to be applied (for which we
 * have to set a coefficient and a modul). Finally, we want the rank to be
 * stored at the nodes (as an uint8) instead of being re-computed every time.
 */
using MyTreeOptions =
    TreeOptions<TreeFlags::ZTREE_USE_HASH,
                TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_MODUL<42>,
                TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT<1701>,
                TreeFlags::ZTREE_RANK_TYPE<uint8_t>>;

/* The node class
 * Provides a simple key -> value mapping, where the key is an integer and the
 * value is a string.
 */
class Node : public ZTreeNodeBase<Node, MyTreeOptions> {
public:
  int key;
  std::string value;

  /* Because we use rank-by-hash in conjunction with storing the rank, we must
   * call update_rank() before using the node. We do this in the constructor,
   * for which we need the key to be set. Thus, an "empty" construction is not
   * allowed. */
  Node() = delete;
  Node(int key_in, std::string value_in)
    : key(key_in), value(value_in)
  {
    // This is important! See the comment above.
    this->update_rank();
  }

  // need to implement this s.t. we can use the default
  // ygg::utilities::flexible_less as comparator
  bool
  operator<(const Node & other) const
  {
    return this->key < other.key;
  }
};

/* For rank-by-hash to work, we need to implement std::hash<Node> */
namespace std {
  template<>
  struct hash<Node> {
    size_t operator()(const Node & n) const {
      return std::hash<int>{}(n.key);
    }
  };
}

// Configure the Zip Tree based on Node and the default NodeTraits
using MyTree = ZTree<Node, ZTreeDefaultNodeTraits<Node>, MyTreeOptions>;

// We need this s.t. we can query by key value (i.e, an int) directly
bool
operator<(const Node & lhs, const int rhs)
{
  return lhs.key < rhs;
}
bool
operator<(const int lhs, const Node & rhs)
{
  return lhs < rhs.key;
}

int
main(int argc, char ** argv)
{
  (void)argc;
  (void)argv;

  MyTree t;

  // Storage for the actual nodes.
  // WARNING: using STL containers here can backfire badly. See TODO.
  std::vector<Node> nodes;

  // Initialize the nodes with some values
  for (int i = 0; i < 5; ++i) {
    nodes.emplace_back(i, std::string("The key is ") + std::to_string(i));
  }

  // Insert them
  for (size_t i = 0; i < 5; ++i) {
    t.insert(nodes[i]);
  }

  // What was the string for i = 3 again?
  auto it = t.find(3); // Note we're using a int to query here, not a Node
  assert(it != t.end());
  std::string retrieved_value = it->value; // *it is the Node

  // Okay, we don't need that Node anymore.
  t.remove(*it);

  return 0;
}
