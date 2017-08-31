#include "../src/intervaltree.hpp"

#include <random>
#include <set>
#include <iostream>
#include <chrono>
#include <functional>

using hrc = std::chrono::high_resolution_clock;

using namespace ygg;

#define BENCHMARK_SIZE 5000000

class Node : public RBTreeNodeBase<Node, TreeOptions<>> {
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
  using Tree = RBTree<Node, NodeTraits, TreeOptions<>>;

  Profiler() {
    this->nodes = new Node[BENCHMARK_SIZE];
  };

  void run()
  {
    this->linearize_nodes();
    this->insert_linear();
    this->profile();
  }

private:
  void profile() {
    for (auto j = 0 ; j < 50 ; ++j) {
      for (auto i = 0 ; i < BENCHMARK_SIZE ; ++i) {
        volatile auto it = t.find(nodes[i]);
      }
    }
  }

  void insert_linear() {
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
  Tree t;
};


int main()
{
  Profiler b;

  b.run();
}
