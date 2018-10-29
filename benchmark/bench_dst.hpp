#ifndef YGG_BENCH_DST_HPP
#define YGG_BENCH_DST_HPP

#include <celero/Celero.h>
#include <cmath>
#include <random>
#include <set>
#include <vector>

#include "../src/ygg.hpp"
#include "config.hpp"

#ifdef PAPI_FOUND
#include <papi.h>
#endif

using namespace ygg;

constexpr size_t TEST_SIZES = 13;
constexpr size_t TEST_SIZE_BASE_EXPONENT = 2;

class PAPIMeasurement : public celero::UserDefinedMeasurementTemplate<size_t> {
public:
  PAPIMeasurement(std::string name_in) : name(name_in) {}

  virtual std::string
  getName() const override
  {
    return std::string(this->name);
  }

  virtual bool
  reportSize() const override
  {
    return false;
  };
  virtual bool
  reportMean() const override
  {
    return true;
  };
  virtual bool
  reportVariance() const override
  {
    return false;
  };
  virtual bool
  reportStandardDeviation() const override
  {
    return true;
  };
  virtual bool
  reportSkewness() const override
  {
    return false;
  };
  virtual bool
  reportKurtosis() const override
  {
    return false;
  };
  virtual bool
  reportZScore() const override
  {
    return false;
  };
  virtual bool
  reportMin() const override
  {
    return false;
  };
  virtual bool
  reportMax() const override
  {
    return false;
  };

private:
  std::string name;
};

template <class Selector, class CombinerPack>
class DynSegTreeBaseFixture : public celero::TestFixture {

public:
  DynSegTreeBaseFixture()
  {
    this->instance_sizes.clear();

    const auto & configured_sizes = Cmdline::get().get_sizes();
    if (configured_sizes.empty()) {
      for (size_t i = 0; i < TEST_SIZES; i++) {
	this->instance_sizes.push_back(
	    {static_cast<int32_t>(std::pow(2, (i + TEST_SIZE_BASE_EXPONENT)))});
      }
    } else {
      for (auto & value_pair : configured_sizes) {
	this->instance_sizes.push_back(
	    {static_cast<int32_t>(value_pair.first),
	     static_cast<int32_t>(value_pair.second)});
      }
    }

#ifdef PAPI_FOUND
    if (PAPI_is_initialized() == PAPI_NOT_INITED) {
      PAPI_library_init(PAPI_VER_CURRENT);
    }

    const std::vector<std::string> papi_requested =
        Cmdline::get().get_papi_types();
    this->selected_events.clear();
    int event_code;
    event_code = 0 | PAPI_NATIVE_MASK;

    for (const std::string & event_str : papi_requested) {
      auto ret = PAPI_event_name_to_code(event_str.c_str(), &event_code);
      if (ret != PAPI_OK) {
	std::cerr << "PAPI event " << event_str
	          << " not found! Error Code: " << ret << "\n";
	exit(-1);
      }

      this->papi_names.push_back(event_str);
      this->selected_events.push_back(event_code);
      this->papi_measurements.emplace_back(new PAPIMeasurement(event_str));
    }

    int num_counters = PAPI_num_counters();
    if (num_counters < this->selected_events.size()) {
      std::cerr << "Too few PAPI counters available.\n";
      exit(-1);
    }

    this->event_counts.resize(this->selected_events.size(), 0);

    // Test PAPI
    if (!this->selected_events.empty()) {
      auto ret = PAPI_start_counters(this->selected_events.data(),
                                     (int)this->selected_events.size());
      if (ret != PAPI_OK) {
	std::cerr << "Could not start PAPI measurements. Error code: " << ret
	          << "\n";
	exit(-1);
      }
      if (PAPI_stop_counters(this->event_counts.data(),
                             (int)this->event_counts.size()) != PAPI_OK) {
	std::cerr << "Could not stop PAPI measurements. Error code: " << ret
	          << "\n";
	exit(-1);
      }
    }
#endif
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
    std::uniform_real_distribution<double> val_distr(0, 20);
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

      //      std::cout << "Node: [" << this->nodes[i].lower << ", " <<
      //      this->nodes[i].upper << "): " << this->nodes[i].val << "\n";
    }

#ifdef PAPI_FOUND
    if (!this->selected_events.empty()) {
      PAPI_start_counters(this->selected_events.data(),
                          (int)this->selected_events.size());
    }
#endif
  }

  virtual void
  tearDown() override
  {
#ifdef PAPI_FOUND
    if (!this->selected_events.empty()) {
      PAPI_stop_counters(this->event_counts.data(),
                         (int)this->event_counts.size());
      for (size_t i = 0; i < this->selected_events.size(); ++i) {
	this->papi_measurements[i]->addValue(this->event_counts[i]);
      }
    }
#endif

    this->nodes.clear();
  }

  virtual std::vector<std::shared_ptr<celero::UserDefinedMeasurement>>
  getUserDefinedMeasurements() const override
  {
#ifdef PAPI_FOUND
    std::vector<std::shared_ptr<celero::UserDefinedMeasurement>> ret;
    for (auto & ptr : this->papi_measurements) {
      ret.push_back(ptr);
    }
    return ret;
#else
    return {};
#endif
  }

  class Node
      : public DynSegTreeNodeBase<int, double, double, CombinerPack, Selector> {
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

  /* PAPI measurement */
  std::vector<std::shared_ptr<PAPIMeasurement>> papi_measurements;
  std::vector<std::string> papi_names;
  std::vector<int> selected_events;
  std::vector<long long int> event_counts;
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

using RBBaseFixture =
    DynSegTreeBaseFixture<UseRBTree, EmptyCombinerPack<int, double>>;
using ZTBaseFixture =
    DynSegTreeBaseFixture<UseZipTree, EmptyCombinerPack<int, double>>;
using RBFilledFixture =
    DynSegTreeFilledFixture<UseRBTree, EmptyCombinerPack<int, double>>;
using ZTreeFilledFixture =
    DynSegTreeFilledFixture<UseZipTree, EmptyCombinerPack<int, double>>;
using RBMoveFixture =
    DynSegTreeMoveFixture<UseRBTree, EmptyCombinerPack<int, double>>;
using ZTreeMoveFixture =
    DynSegTreeMoveFixture<UseZipTree, EmptyCombinerPack<int, double>>;

/*******************************************
 * Benchmark 1: Moving
 *******************************************
 */
BENCHMARK_F(Move, RBDST, RBMoveFixture, 10, 1)
{
  for (size_t i = 0; i < this->node_count; ++i) {
    this->t.remove(this->nodes[i]);
    this->nodes[i].lower = this->new_positions[i].first;
    this->nodes[i].upper = this->new_positions[i].second;
    this->t.insert(this->nodes[i]);
  }
}

BASELINE_F(Move, ZTDST, ZTreeMoveFixture, 10, 1)
{
  for (size_t i = 0; i < this->node_count; ++i) {
    this->t.remove(this->nodes[i]);
    this->nodes[i].lower = this->new_positions[i].first;
    this->nodes[i].upper = this->new_positions[i].second;
    this->t.insert(this->nodes[i]);
  }
}

#endif
