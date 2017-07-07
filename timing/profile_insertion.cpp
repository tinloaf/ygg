#include "../src/intervaltree.hpp"

#include <random>
#include <set>
#include <iostream>
#include <chrono>
#include <functional>

using hrc = std::chrono::high_resolution_clock;

using namespace ygg;

#define BENCHMARK_SIZE 5000000

class Node : public RBTreeNodeBase<Node, false> {
public:
  int data;

  Node () : data(0) {};
  explicit Node(int data_in) : data(data_in) {};
  Node(const Node &other) : data(other.data) {};

  bool operator<(const Node & other) const {
    return other.data < this->data;
  }
};

class NodeTraits : public RBDefaultNodeTraits<Node> {
public:
  static std::string get_id(const Node * node) {
    return std::to_string(node->data);
  }
};

class Profiler {
public:
  using Tree = RBTree<Node, NodeTraits>;

  Profiler() {
    this->nodes = new Node[BENCHMARK_SIZE];
  };

  void run()
  {
    this->linearize_nodes();
    this->profile();
  }

private:
  void profile() {
    Tree t;
    for (auto i = 0 ; i < BENCHMARK_SIZE ; ++i) {
      t.insert(nodes[i]);
    }
  }

  void linearize_nodes()
  {
    for (auto i = 0 ; i < BENCHMARK_SIZE ; ++i) {
      nodes[i] = Node(i);
    }
  }

  Node * nodes;
};


int main()
{
  Profiler b;

  b.run();
}
