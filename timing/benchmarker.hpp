#include "../src/intervaltree.hpp"

#include <random>
#include <set>
#include <iostream>
#include <chrono>
#include <boost/intrusive/set.hpp>
#include <functional>

using hrc = std::chrono::high_resolution_clock;

using namespace ygg;

#define BENCHMARK_SIZE 5000000

class Node : public RBTreeNodeBase<Node, false>, public boost::intrusive::set_base_hook<> {
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

class Benchmarker {
public:
  using Tree = RBTree<Node, NodeTraits>;
  using BoostSet = boost::intrusive::set< Node, boost::intrusive::compare<std::less<Node> > >;

  Benchmarker() {
    this->nodes = new Node[BENCHMARK_SIZE];
  };

  void run_all()
  {
    std::cout << "\n\n== Linear Insertion Order\n";
    this->linearize_nodes();
    this->benchmark_insertion();
    std::cout << "\n=== Linear Query Order\n";
    this->linearize_query_order();
    this->benchmark_queries();
    std::cout << "\n=== Random Query Order\n";
    this->randomize_query_order();
    this->benchmark_queries();

    std::cout << "\n\n== Random Insertion Order\n";
    this->randomize_nodes();
    this->benchmark_insertion();
    std::cout << "\n=== Linear Query Order\n";
    this->linearize_query_order();
    this->benchmark_queries();
    std::cout << "\n=== Random Query Order\n";
    this->randomize_query_order();
    this->benchmark_queries();
  }

private:

  void benchmark_queries() {
    std::cout << "====== Queries\n";

    //
    // IntervalTree
    //
    std::cout << "IntervalTree: ";

    Tree t;
    for (auto i = 0 ; i < BENCHMARK_SIZE ; ++i) {
      t.insert(nodes[i]);
    }

    this->flush_cache();

    hrc::time_point before_it = hrc::now();
    for (auto index : this->query_order) {
      auto it = t.find(nodes[index]);
    }
    hrc::time_point after_it = hrc::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( after_it - before_it ).count();
    std::cout << "\t\t\t" << duration << "\n";
    double base_duration = duration;

    //
    // Boost
    //

    std::cout << "boost::intrusive::set: ";

    BoostSet bset;
    for (auto i = 0 ; i < BENCHMARK_SIZE ; ++i) {
      bset.insert(nodes[i]);
    }

    this->flush_cache();

    auto before_bset = hrc::now();
    for (auto index : this->query_order) {
      auto it = bset.find(nodes[index]);
    }
    auto after_bset = hrc::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>( after_bset - before_bset ).count();

    std::cout << "\t\t" << duration << "(" << duration / base_duration << ")" << "\n";

    //
    // IntervalTree
    //
    std::cout << "IntervalTree: ";

    t.clear();
    for (auto i = 0 ; i < BENCHMARK_SIZE ; ++i) {
      t.insert(nodes[i]);
    }

    this->flush_cache();

    before_it = hrc::now();
    for (auto index : this->query_order) {
      auto it = t.find(nodes[index]);
    }
    after_it = hrc::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>( after_it - before_it ).count();
    std::cout << "\t\t\t" << duration << "(" << duration / base_duration << ")" << "\n";

    //
    // std::set
    //

    std::cout << "std::set: ";

    std::set<Node> set;
    for (auto i = 0 ; i < BENCHMARK_SIZE ; ++i) {
      set.insert(nodes[i]);
    }

    this->flush_cache();

    auto before_set = hrc::now();
    for (auto index : this->query_order) {
      auto it = set.find(nodes[index]);
    }
    auto after_set = hrc::now();

    duration = std::chrono::duration_cast<std::chrono::microseconds>( after_set - before_set ).count();

    std::cout << "\t\t\t" << duration << "(" << duration / base_duration << ")" << "\n";
  }

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
    double base_duration = duration;

    std::cout << "\t\t\t" << duration << "\n";

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

    std::cout << "\t\t\t" << duration << "(" << duration / base_duration << ")" << "\n";

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

    std::cout << "\t\t" << duration << "(" << duration / base_duration << ")" << "\n";


    this->flush_cache();

    //
    // IntervalTree
    //

    std::cout << "IntervalTree: ";

    t.clear();
    before_it = hrc::now();
    for (auto i = 0 ; i < BENCHMARK_SIZE ; ++i) {
      t.insert(nodes[i]);
    }
    after_it = hrc::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>( after_it - before_it ).count();

    std::cout << "\t\t\t" << duration << "(" << duration / base_duration << ")" << "\n";
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

  void linearize_query_order()
  {
    this->query_order.clear();
    for (auto i = 0 ; i < BENCHMARK_SIZE ; ++i) {
      this->query_order.push_back(i);
    }
  }

  void randomize_query_order()
  {
    this->linearize_query_order();

    std::mt19937 rng(4); // chosen by fair xkcd
    std::random_shuffle(this->query_order.begin(), this->query_order.end(), [&](int i) {
      std::uniform_int_distribution<unsigned int> uni(0,
                                             i - 1);
      return uni(rng);
    });
  }

  Node * nodes;
  std::vector<size_t> query_order;
};
