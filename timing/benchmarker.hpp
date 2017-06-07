#include "../src/intervaltree.hpp"

#include <random>
#include <set>
#include <iostream>
#include <chrono>
#include <boost/intrusive/set.hpp>
#include <functional>

using hrc = std::chrono::high_resolution_clock;

#define BENCHMARK_SIZE 100000

class Node : public IntervalTreeNodeBase<Node>, public boost::intrusive::set_base_hook<> {
public:
  int data;

  Node () : data(0) {};
  explicit Node(int data_in) : data(data_in) {};
  Node(const Node &other) : data(other.data) {};

  bool operator<(const Node & other) const {
    return other.data < this->data;
  }
};

class NodeTraits {
public:
  static int get_lower(const Node * node) {
    return node->data;
  }

  static std::string get_id(const Node * node) {
    return std::to_string(node->data);
  }
};

class Benchmarker {
public:
  using Tree = IntervalTree<Node, NodeTraits>;
  using BoostSet = boost::intrusive::set< Node, boost::intrusive::compare<std::less<Node> > >;

  void run_all()
  {
    std::cout << "== Linear Order\n";
    this->linearize_nodes();
    this->benchmark_insertion();

    std::cout << "== Random Order\n";
    this->linearize_nodes();
    this->benchmark_insertion();
  }

private:

  void benchmark_insertion() {
    std::cout << "====== Insertion\n";
    this->flush_cache();

    //
    // IntervalTree
    //

    std::cout << "IntervalTree: ";

    Tree t;
    hrc::time_point before_it = hrc::now();
    for (auto i = 0 ; i < BENCHMARK_SIZE ; ++i) {
      t.insert(nodes[i]);
    }
    hrc::time_point after_it = hrc::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( after_it - before_it ).count();

    std::cout << duration << "\n";

    this->flush_cache();

    //
    // std::set
    //

    std::cout << "std::set: ";

    std::set<Node> set;
    hrc::time_point before_set = hrc::now();
    for (auto i = 0 ; i < BENCHMARK_SIZE ; ++i) {
      set.insert(nodes[i]);
    }
    hrc::time_point after_set = hrc::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>( after_set - before_set ).count();

    std::cout << duration << "\n";

    //
    // Boost
    //

    std::cout << "boost::intrusive::set: ";

    BoostSet bset;
    hrc::time_point before_bset = hrc::now();
    for (auto i = 0 ; i < BENCHMARK_SIZE ; ++i) {
      bset.insert(nodes[i]);
    }
    hrc::time_point after_bset = hrc::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>( after_bset - before_bset ).count();

    std::cout << duration << "\n";
  }

  void flush_cache()
  {
    for (auto i = 0 ; i < BENCHMARK_SIZE ; ++i) {
      asm volatile ("clflush (%0)" :: "r"(&nodes[i]));
    }
  }

  void randomize_nodes()
  {
    std::mt19937 rng(4); // chosen by fair xkcd
    std::uniform_int_distribution<int> uni(std::numeric_limits<int>::min(),
                                           std::numeric_limits<int>::max());

    for (auto i = 0 ; i < BENCHMARK_SIZE ; ++i) {
      nodes[i] = Node(uni(rng));
    }
  }

  void linearize_nodes()
  {
    for (auto i = 0 ; i < BENCHMARK_SIZE ; ++i) {
      nodes[i] = Node(i);
    }
  }

  Node nodes[BENCHMARK_SIZE];
};
