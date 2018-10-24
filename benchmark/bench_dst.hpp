#ifndef YGG_BENCH_DST_HPP
#define YGG_BENCH_DST_HPP

#include <celero/Celero.h>
#include <cmath>
#include <random>
#include <set>
#include <vector>

#include "../src/ygg.hpp"

using namespace ygg;

constexpr size_t TEST_SIZES = 10;
constexpr size_t TEST_SIZE_BASE_EXPONENT = 2;

template <class Selector, class CombinerPack>
class DynSegTreeBaseFixture : public celero::TestFixture {

public:
  DynSegTreeBaseFixture()
  {
    this->instance_sizes.clear();
    for (size_t i = 0; i < TEST_SIZES; i++) {
      this->instance_sizes.push_back(
          {static_cast<int32_t>(std::pow(2, (i + TEST_SIZE_BASE_EXPONENT)))});
    }
  }

  virtual std::vector<celero::TestFixture::ExperimentValue>
  getExperimentValues() const override
  {
    return this->instance_sizes;
  };

  virtual void
  setUp(const celero::TestFixture::ExperimentValue & node_count_in) override
  {
    //    std::cout << "====== New experiment ===========\n";
    
    this->node_count = node_count_in.Value;
    this->nodes.resize((size_t)this->node_count);

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<int> point_distr(
        0, std::numeric_limits<int>::max());
    std::uniform_real_distribution<double> val_distr(
        0, 20);
    for (size_t i = 0; i < (size_t)this->node_count; ++i) {
      int p1 = point_distr(gen);
      int p2 = point_distr(gen);

      if (p1 < p2) {
	this->nodes[i].lower = p1;
	this->nodes[i].upper = p2;
      } else if (p2 < p1) {
	this->nodes[i].lower = p2;
	this->nodes[i].upper = p1;
      } else {
	this->nodes[i].lower = p1;
	this->nodes[i].upper = p1 + 1;
      }

      this->nodes[i].val = val_distr(gen);

      //      std::cout << "Node: [" << this->nodes[i].lower << ", " << this->nodes[i].upper << "): " << this->nodes[i].val << "\n";
    }
  }

  virtual void
  tearDown() override
  {
    this->nodes.clear();
  }

  class Node : public DynSegTreeNodeBase<int, double, double, CombinerPack, Selector> {
  public:
    int lower;
    int upper;
    double val;
  };

  class NodeTraits : public DynSegTreeNodeTraits<Node> {
  public:
    static int
    get_lower(const Node & n)
    {
      return n.lower;
    }
    static int
    get_upper(const Node & n)
    {
      return n.upper;
    }
    static double
    get_value(const Node & n)
    {
      return n.val;
    }
  };

  using Tree = DynamicSegmentTree<Node, NodeTraits, CombinerPack,
                                  DefaultOptions, Selector>;

  Tree t;

  std::vector<celero::TestFixture::ExperimentValue> instance_sizes;

  std::vector<Node> nodes;
  int64_t node_count;
};

template <class Selector, class CombinerPack>
class DynSegTreeFilledFixture
    : public DynSegTreeBaseFixture<Selector, CombinerPack> {
public:
  virtual void
  setUp(const celero::TestFixture::ExperimentValue & number_of_nodes) override
  {
    this->DynSegTreeBaseFixture<Selector, CombinerPack>::setUp(number_of_nodes);

    for (auto & n : this->nodes) {
      //      std::cout << "Inserting node at " << (size_t) &n << " \n";
      this->t.insert(n);
      this->t.dbg_verify();
    }
  }
};

template <class Selector, class CombinerPack>
class DynSegTreeMoveFixture
    : public DynSegTreeFilledFixture<Selector, CombinerPack> {
public:
  virtual void
  setUp(const celero::TestFixture::ExperimentValue & number_of_nodes) override
  {
    this->DynSegTreeFilledFixture<Selector, CombinerPack>::setUp(
        number_of_nodes);

    /* New positions */
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<int> point_distr(
        0, std::numeric_limits<int>::max());
    for (size_t i = 0; i < (size_t)this->node_count; ++i) {
      int p1 = point_distr(gen);
      int p2 = point_distr(gen);

      if (p1 < p2) {
	this->new_positions.emplace_back(p1, p2);
      } else if (p2 < p1) {
	this->new_positions.emplace_back(p2, p1);
      } else {
	this->new_positions.emplace_back(p1, p1 + 1);
      }
    }
  }

  std::vector<std::pair<int, int>> new_positions;
};

/*
 * Various actual fixtures
 */

using RBBaseFixture = DynSegTreeBaseFixture<UseRBTree, EmptyCombinerPack<int, double>>;
using ZTBaseFixture = DynSegTreeBaseFixture<UseZipTree, EmptyCombinerPack<int, double>>;
using RBFilledFixture = DynSegTreeFilledFixture<UseRBTree, EmptyCombinerPack<int, double>>;
using ZTreeFilledFixture = DynSegTreeFilledFixture<UseZipTree, EmptyCombinerPack<int, double>>;
using RBMoveFixture = DynSegTreeMoveFixture<UseRBTree, EmptyCombinerPack<int, double>>;
using ZTreeMoveFixture = DynSegTreeMoveFixture<UseZipTree, EmptyCombinerPack<int, double>>;


/*******************************************
 * Benchmark 1: Moving
 *******************************************
 */
BENCHMARK_F(Move, RBDST, RBMoveFixture, 10, 1)
{
  for (size_t i = 0 ; i < this->node_count ; ++i) {
    this->t.remove(this->nodes[i]);
    this->nodes[i].lower = this->new_positions[i].first;
    this->nodes[i].upper = this->new_positions[i].second;
    this->t.insert(this->nodes[i]);
  }
}

BASELINE_F(Move, ZTDST, ZTreeMoveFixture, 10, 1)
{
  for (size_t i = 0 ; i < this->node_count ; ++i) {
    this->t.remove(this->nodes[i]);
    this->nodes[i].lower = this->new_positions[i].first;
    this->nodes[i].upper = this->new_positions[i].second;
    this->t.insert(this->nodes[i]);
  }
}


#endif
