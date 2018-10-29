//
// Created by lukas on 01.09.17.
//

#ifndef YGG_BENCH_RBTREE_HPP
#define YGG_BENCH_RBTREE_HPP

#include "cmdline.hpp"
#include <celero/Celero.h>
#include <cmath>
#include <string>

#include "../benchmark/config.hpp"
#include "../src/ygg.hpp"

#include <boost/intrusive/set.hpp>

#ifdef PAPI_FOUND
#include <papi.h>
#endif

using namespace ygg;

constexpr size_t TEST_SIZES = 9;
constexpr size_t TEST_SIZE_BASE_EXPONENT = 7;

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

template <bool distinct>
class RBTreeBaseFixture : public celero::TestFixture {
public:
  RBTreeBaseFixture()
  {
    this->instance_sizes.clear();
    for (size_t i = 0; i < TEST_SIZES; i++) {
      this->instance_sizes.push_back(
          {static_cast<int32_t>(std::pow(2, (i + TEST_SIZE_BASE_EXPONENT)))});
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
				std::cerr << "PAPI event " << event_str << " not found! Error Code: "<< ret << "\n";
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
  setUp(const celero::TestFixture::ExperimentValue & value_count_in) override
  {
    this->value_count = value_count_in.Value;
    this->values.resize((size_t)this->value_count);

    if (distinct) {
      std::set<int> seen;

      for (size_t i = 0; i < (size_t)this->value_count; ++i) {
	int val = rand();

	while (seen.find(val) != seen.end()) {
	  val = rand();
	}

	this->values[i] = val;
	seen.insert(val);
      }
    } else {
      for (size_t i = 0; i < (size_t)this->value_count; ++i) {
	int val = rand();
	this->values[i] = val;
      }
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

    this->values.clear();
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

  std::vector<celero::TestFixture::ExperimentValue> instance_sizes;

  std::vector<int> values;
  int64_t value_count;

  /* PAPI measurement */
  std::vector<std::shared_ptr<PAPIMeasurement>> papi_measurements;
  std::vector<std::string> papi_names;
  std::vector<int> selected_events;
  std::vector<long long int> event_counts;
};

/*
 * std::set fixtures
 */
class StdSetBaseFixture : public RBTreeBaseFixture<true> {
public:
  virtual void
  setUp(const celero::TestFixture::ExperimentValue & number_of_nodes) override
  {
    this->RBTreeBaseFixture<true>::setUp(number_of_nodes.Value);

    for (int i = 0; i < number_of_nodes.Value; ++i) {
      std::set<int> donor;
      donor.insert(this->values[i]);
      this->nodes.push_back(donor.extract(donor.begin()));
    }
  }

  virtual void
  tearDown() override
  {
    this->RBTreeBaseFixture<true>::tearDown();

    this->s.clear();
  }

  std::vector<decltype(std::set<int>().extract(std::set<int>().begin()))> nodes;
  std::set<int> s;
};

class StdSetInsertFixture : public StdSetBaseFixture {
};

class StdSetSearchFixture : public StdSetBaseFixture {
public:
  virtual void
  setUp(const celero::TestFixture::ExperimentValue & number_of_nodes) override
  {
    this->StdSetBaseFixture::setUp(number_of_nodes.Value);

    for (auto & n : this->nodes) {
      this->s.insert(std::move(n));
    }

    for (int v : this->values) {
      this->search_values.push_back(v);
    }

    this->nodes.clear();
    std::random_shuffle(this->search_values.begin(), this->search_values.end());
  }

  virtual void
  tearDown() override
  {
    this->search_values.clear();

    this->StdSetBaseFixture::tearDown();
  }

  std::vector<int> search_values;
};

/*
 * Ygg's RBTree fixtures
 */
template <class MyTreeOptions>
class YggRBTreeBaseFixture
    : public RBTreeBaseFixture<!MyTreeOptions::multiple> {
public:
  class Node : public RBTreeNodeBase<Node, MyTreeOptions> {
  private:
    int value;

  public:
    void
    set_value(int new_value)
    {
      this->value = new_value;
    }

    int
    get_value() const
    {
      return this->value;
    }

    bool
    operator<(const Node & rhs) const
    {
      return this->value < rhs.value;
    }
  };

  using Tree = RBTree<Node, RBDefaultNodeTraits<Node>, MyTreeOptions>;

  virtual void
  setUp(const celero::TestFixture::ExperimentValue & number_of_nodes) override
  {
    this->RBTreeBaseFixture<!MyTreeOptions::multiple>::setUp(
        number_of_nodes.Value);

    this->nodes.resize((size_t)number_of_nodes.Value);
    for (size_t i = 0; i < (size_t)number_of_nodes.Value; ++i) {
      this->nodes[i].set_value(this->values[i]);
    }
  }

  virtual void
  tearDown() override
  {
    this->RBTreeBaseFixture<!MyTreeOptions::multiple>::tearDown();

    this->nodes.clear();
  }

  std::vector<Node> nodes;
  Tree t;
};

template <class MyTreeOptions>
class YggMultiRBTreeBaseFixture : public RBTreeBaseFixture<false> {
public:
  class Node : public RBTreeNodeBase<Node, TreeOptions<TreeFlags::MULTIPLE>> {
  public:
    int value;

    bool
    operator<(const Node & rhs) const
    {
      return this->value < rhs.value;
    }
  };

  using Tree =
      RBTree<Node, RBDefaultNodeTraits<Node>, TreeOptions<TreeFlags::MULTIPLE>>;

  virtual void
  setUp(const celero::TestFixture::ExperimentValue & number_of_nodes) override
  {
    this->RBTreeBaseFixture<!MyTreeOptions::multiple>::setUp(
        number_of_nodes.Value);

    this->nodes.resize((size_t)number_of_nodes.Value);
    for (size_t i = 0; i < (size_t)number_of_nodes.Value; ++i) {
      this->nodes[i].set_value(this->values[i]);
    }
  }

  virtual void
  tearDown() override
  {
    this->RBTreeBaseFixture<!MyTreeOptions::multiple>::tearDown();

    this->nodes.clear();
  }

  std::vector<Node> nodes;
  Tree t;
};

template <class MyTreeOptions>
class YggRBTreeInsertFixture : public YggRBTreeBaseFixture<MyTreeOptions> {
};

template <class MyTreeOptions>
class YggRBTreeSearchFixture : public YggRBTreeBaseFixture<MyTreeOptions> {
public:
  virtual void
  setUp(const celero::TestFixture::ExperimentValue & number_of_nodes) override
  {
    this->YggRBTreeBaseFixture<MyTreeOptions>::setUp(number_of_nodes.Value);

    for (auto & n : this->nodes) {
      this->t.insert(n);
      this->search_values.push_back(&n);
    }

    std::random_shuffle(this->search_values.begin(), this->search_values.end());
  }

  virtual void
  tearDown() override
  {
    this->t.clear();
    this->search_values.clear();

    this->YggRBTreeBaseFixture<MyTreeOptions>::tearDown();
  }

  std::vector<typename YggRBTreeBaseFixture<MyTreeOptions>::Node *>
      search_values;
};

/*
 * Ygg's ZipTree fixtures
 */
template <class Options>
class ZipNode : public ZTreeNodeBase<ZipNode<Options>, Options> {
private:
  int value;

public:
  void
  set_value(int new_value)
  {
    this->value = new_value;
    this->update_rank();
  }

  int
  get_value() const
  {
    return this->value;
  }

  bool
  operator<(const ZipNode<Options> & rhs) const
  {
    return this->value < rhs.value;
  }
};

template <class MyTreeOptions>
class YggZipTreeBaseFixture : public RBTreeBaseFixture<false> {
public:
  using Node = ZipNode<MyTreeOptions>;
  using Tree = ZTree<Node, ZTreeDefaultNodeTraits<Node>, MyTreeOptions>;

  virtual void
  setUp(const celero::TestFixture::ExperimentValue & number_of_nodes) override
  {
    this->RBTreeBaseFixture<false>::setUp(number_of_nodes.Value);

    this->nodes.resize((size_t)number_of_nodes.Value);
    for (size_t i = 0; i < (size_t)number_of_nodes.Value; ++i) {
      this->nodes[i].set_value(this->values[i]);
    }
  }

  virtual void
  tearDown() override
  {
    this->RBTreeBaseFixture<false>::tearDown();

    this->nodes.clear();
  }

  std::vector<Node> nodes;
  Tree t;
};

// Necessary for rank-by-hash
namespace std {
template <class T>
struct hash<ZipNode<T>>
{
  size_t
  operator()(const ZipNode<T> & n) const
  {
    return n.get_value();
  }
};

} // namespace std

template <class MyTreeOptions>
class YggZipTreeInsertFixture : public YggZipTreeBaseFixture<MyTreeOptions> {
};

template <class MyTreeOptions>
class YggZipTreeSearchFixture : public YggZipTreeBaseFixture<MyTreeOptions> {
public:
  virtual void
  setUp(const celero::TestFixture::ExperimentValue & number_of_nodes) override
  {
    this->YggZipTreeBaseFixture<MyTreeOptions>::setUp(number_of_nodes.Value);

    for (auto & n : this->nodes) {
      this->t.insert(n);
      this->search_values.push_back(&n);
    }

    std::random_shuffle(this->search_values.begin(), this->search_values.end());
  }

  virtual void
  tearDown() override
  {
    this->t.clear();
    this->search_values.clear();

    this->YggZipTreeBaseFixture<MyTreeOptions>::tearDown();
  }

  std::vector<typename YggZipTreeBaseFixture<MyTreeOptions>::Node *>
      search_values;
};

/*
 * Boost fixtures
 */

class BoostSetBaseFixture : public RBTreeBaseFixture<true> {
public:
  class Node
      : public boost::intrusive::set_base_hook<boost::intrusive::link_mode<
            boost ::intrusive::link_mode_type::normal_link>> {
  public:
    int value;

    bool
    operator<(const Node & rhs) const
    {
      return this->value < rhs.value;
    }
  };

  using Tree = boost::intrusive::set<Node>;

  virtual void
  setUp(const celero::TestFixture::ExperimentValue & number_of_nodes) override
  {
    this->RBTreeBaseFixture<true>::setUp(number_of_nodes.Value);

    this->nodes.resize((size_t)number_of_nodes.Value);
    for (size_t i = 0; i < (size_t)number_of_nodes.Value; ++i) {
      this->nodes[i].value = this->values[i];
    }
  }

  virtual void
  tearDown() override
  {
    this->RBTreeBaseFixture<true>::tearDown();

    this->nodes.clear();
  }

  std::vector<Node> nodes;
  Tree t;
};

class BoostMultiSetBaseFixture : public RBTreeBaseFixture<false> {
public:
  class Node
      : public boost::intrusive::set_base_hook<boost::intrusive::link_mode<
            boost ::intrusive::link_mode_type::normal_link>> {
  public:
    int value;

    bool
    operator<(const Node & rhs) const
    {
      return this->value < rhs.value;
    }
  };

  using Tree = boost::intrusive::multiset<Node>;

  virtual void
  setUp(const celero::TestFixture::ExperimentValue & number_of_nodes) override
  {
    this->RBTreeBaseFixture<false>::setUp(number_of_nodes.Value);

    this->nodes.resize((size_t)number_of_nodes.Value);
    for (size_t i = 0; i < (size_t)number_of_nodes.Value; ++i) {
      this->nodes[i].value = this->values[i];
    }
  }

  virtual void
  tearDown() override
  {
    this->RBTreeBaseFixture<false>::tearDown();

    this->nodes.clear();
  }

  std::vector<Node> nodes;
  Tree t;
};

class BoostSetInsertFixture : public BoostSetBaseFixture {
};

class BoostSetSearchFixture : public BoostSetBaseFixture {
public:
  virtual void
  setUp(const celero::TestFixture::ExperimentValue & number_of_nodes) override
  {
    this->BoostSetBaseFixture::setUp(number_of_nodes.Value);

    for (auto & n : this->nodes) {
      this->t.insert(n);
      this->search_values.push_back(&n);
    }

    std::random_shuffle(this->search_values.begin(), this->search_values.end());
  }

  virtual void
  tearDown() override
  {
    this->t.clear();
    this->search_values.clear();

    this->BoostSetBaseFixture::tearDown();
  }

  std::vector<Node *> search_values;
};

class BoostMultiSetInsertFixture : public BoostMultiSetBaseFixture {
};

class BoostMultiSetSearchFixture : public BoostMultiSetBaseFixture {
public:
  virtual void
  setUp(const celero::TestFixture::ExperimentValue & number_of_nodes) override
  {
    this->BoostMultiSetBaseFixture::setUp(number_of_nodes.Value);

    for (auto & n : this->nodes) {
      this->t.insert(n);
      this->search_values.push_back(&n);
    }

    std::random_shuffle(this->search_values.begin(), this->search_values.end());
  }

  virtual void
  tearDown() override
  {
    this->t.clear();
    this->search_values.clear();

    this->BoostMultiSetBaseFixture::tearDown();
  }

  std::vector<Node *> search_values;
};

/* ===================================================
 * Actual tests
 * ===================================================
 */

/*
 * Various Parameters for the RBTree / Zip Tree
 */
using BasicTreeOptions = TreeOptions<
    TreeFlags::ZTREE_USE_HASH, TreeFlags::ZTREE_RANK_TYPE<uint8_t>,
    TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT<9859957398433823229ul>,
    TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_MODUL<
        std::numeric_limits<size_t>::max()>>;
using ZTreeNoStoreOptions = TreeOptions<
    TreeFlags::ZTREE_USE_HASH,
    TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT<9859957398433823229ul>,
    TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_MODUL<
        std::numeric_limits<size_t>::max()>>;
using ZTreeRandomOptions = TreeOptions<
    TreeFlags::ZTREE_RANK_TYPE<uint8_t>,
    TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT<9859957398433823229ul>,
    TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_MODUL<
        std::numeric_limits<size_t>::max()>>;
using MultiTreeOptions = TreeOptions<TreeFlags::MULTIPLE>;
using CompressedTreeOptions = TreeOptions<TreeFlags::COMPRESS_COLOR>;

/*
 * Inserting
 */

BASELINE_F(Insert, RBTree, YggRBTreeInsertFixture<BasicTreeOptions>, 1000, 1)
{
  this->t.clear();

  for (auto & n : this->nodes) {
    this->t.insert(n);
  }
  celero::DoNotOptimizeAway(this->t);
}

BENCHMARK_F(Insert, Zip, YggZipTreeInsertFixture<BasicTreeOptions>, 1000, 1)
{
  this->t.clear();

  for (auto & n : this->nodes) {
    this->t.insert(n);
  }
  celero::DoNotOptimizeAway(this->t);
  // this->t.dbg_print_rank_stats();
}

BENCHMARK_F(Insert, NoStoreZip, YggZipTreeInsertFixture<ZTreeNoStoreOptions>,
            1000, 1)
{
  this->t.clear();

  for (auto & n : this->nodes) {
    this->t.insert(n);
  }
  celero::DoNotOptimizeAway(this->t);
  // this->t.dbg_print_rank_stats();
}

BENCHMARK_F(Insert, RandomZip, YggZipTreeInsertFixture<ZTreeRandomOptions>,
            1000, 1)
{
  this->t.clear();

  for (auto & n : this->nodes) {
    this->t.insert(n);
  }
  celero::DoNotOptimizeAway(this->t);
  // this->t.dbg_print_rank_stats();
}

BENCHMARK_F(Insert, StdSet, StdSetInsertFixture, 1000, 1)
{
  for (auto & n : this->nodes) {
    this->s.insert(std::move(n));
  }
  celero::DoNotOptimizeAway(s);
}

BENCHMARK_F(Insert, Boostset, BoostSetInsertFixture, 1000, 1)
{
  this->t.clear();

  for (auto & n : this->nodes) {
    this->t.insert(n);
  }
  celero::DoNotOptimizeAway(this->t);
}

BENCHMARK_F(Insert, MultiYgg, YggRBTreeInsertFixture<MultiTreeOptions>, 1000, 1)
{
  this->t.clear();

  for (auto & n : this->nodes) {
    this->t.insert(n);
  }
  celero::DoNotOptimizeAway(this->t);
}

BENCHMARK_F(Insert, YggValidation, YggRBTreeInsertFixture<BasicTreeOptions>,
            1000, 1)
{
  this->t.clear();

  for (auto & n : this->nodes) {
    this->t.insert(n);
  }
  celero::DoNotOptimizeAway(this->t);
}

BENCHMARK_F(Insert, CompressedYgg,
            YggRBTreeInsertFixture<CompressedTreeOptions>, 1000, 1)
{
  this->t.clear();

  for (auto & n : this->nodes) {
    this->t.insert(n);
  }
  celero::DoNotOptimizeAway(this->t);
}

BENCHMARK_F(Insert, BoostMultiSet, BoostMultiSetInsertFixture, 1000, 1)
{
  this->t.clear();

  for (auto & n : this->nodes) {
    this->t.insert(n);
  }
  celero::DoNotOptimizeAway(this->t);
}

/*
 * Searching
 */

BASELINE_F(Search, RBTree, YggRBTreeSearchFixture<BasicTreeOptions>, 30, 50)
{
  int sum = 0;
  for (auto & v : this->search_values) {
    auto it = this->t.find(*v);
    sum += it->get_value();
  }

  celero::DoNotOptimizeAway(sum);
}

BENCHMARK_F(Search, Zip, YggZipTreeSearchFixture<BasicTreeOptions>, 30, 50)
{
  int sum = 0;
  for (auto & v : this->search_values) {
    auto it = this->t.find(*v);
    sum += it->get_value();
  }

  celero::DoNotOptimizeAway(sum);
}

BENCHMARK_F(Search, NoStoreZip, YggZipTreeSearchFixture<ZTreeNoStoreOptions>,
            30, 50)
{
  int sum = 0;
  for (auto & v : this->search_values) {
    auto it = this->t.find(*v);
    sum += it->get_value();
  }

  celero::DoNotOptimizeAway(sum);
}

BENCHMARK_F(Search, StdSet, StdSetSearchFixture, 30, 50)
{
  int sum = 0;
  for (int v : this->search_values) {
    auto it = this->s.find(v);
    sum += (*it);
  }

  celero::DoNotOptimizeAway(sum);
}

BENCHMARK_F(Search, BoostSet, BoostSetSearchFixture, 30, 50)
{
  int sum = 0;
  for (auto & v : this->search_values) {
    auto it = this->t.find(*v);
    sum += it->value;
  }

  celero::DoNotOptimizeAway(sum);
}

BENCHMARK_F(Search, MultiYgg, YggRBTreeSearchFixture<MultiTreeOptions>, 30, 50)
{
  int sum = 0;
  for (auto & v : this->search_values) {
    auto it = this->t.find(*v);
    sum += it->get_value();
  }

  celero::DoNotOptimizeAway(sum);
}

BENCHMARK_F(Search, CompressedYgg,
            YggRBTreeSearchFixture<CompressedTreeOptions>, 30, 50)
{
  int sum = 0;
  for (auto & v : this->search_values) {
    auto it = this->t.find(*v);
    sum += it->get_value();
  }

  celero::DoNotOptimizeAway(sum);
}

BENCHMARK_F(Search, BoostMultiSet, BoostMultiSetSearchFixture, 30, 50)
{
  int sum = 0;
  for (auto & v : this->search_values) {
    auto it = this->t.find(*v);
    sum += it->value;
  }

  celero::DoNotOptimizeAway(sum);
}

/*
 * Iteration
 */

BASELINE_F(Iteration, RBTree, YggRBTreeSearchFixture<BasicTreeOptions>, 50, 200)
{
  for (const auto & n : this->t) {
    // sum += n.value;
    celero::DoNotOptimizeAway(n);
  }
}

BENCHMARK_F(Iteration, Zip, YggZipTreeSearchFixture<BasicTreeOptions>, 50, 200)
{
  for (const auto & n : this->t) {
    // sum += n.value;
    celero::DoNotOptimizeAway(n);
  }
}

BENCHMARK_F(Iteration, NoStoreZip, YggZipTreeSearchFixture<ZTreeNoStoreOptions>,
            50, 200)
{
  for (const auto & n : this->t) {
    // sum += n.value;
    celero::DoNotOptimizeAway(n);
  }
}

BENCHMARK_F(Iteration, StdSet, StdSetSearchFixture, 50, 200)
{
  for (const auto & n : this->s) {
    // sum += n.value;
    celero::DoNotOptimizeAway(n);
  }
}

BENCHMARK_F(Iteration, BoostSet, BoostSetSearchFixture, 50, 200)
{
  for (const auto & n : this->t) {
    // sum += n.value;
    celero::DoNotOptimizeAway(n);
  }
}

BENCHMARK_F(Iteration, MultiYgg, YggRBTreeSearchFixture<MultiTreeOptions>, 50,
            200)
{
  for (const auto & n : this->t) {
    // sum += n.value;
    celero::DoNotOptimizeAway(n);
  }
}

BENCHMARK_F(Iteration, CompressedYgg,
            YggRBTreeSearchFixture<CompressedTreeOptions>, 50, 200)
{
  for (const auto & n : this->t) {
    // sum += n.value;
    celero::DoNotOptimizeAway(n);
  }
}

BENCHMARK_F(Iteration, BoostMultiSet, BoostMultiSetSearchFixture, 50, 200)
{
  for (const auto & n : this->t) {
    // sum += n.value;
    celero::DoNotOptimizeAway(n);
  }
}

/*
 * Deletion
 */

BASELINE_F(Delete, RBTree, YggRBTreeSearchFixture<BasicTreeOptions>, 1000, 1)
{
  for (auto & v : this->search_values) {
    this->t.remove(*v);
  }

  celero::DoNotOptimizeAway(this->t);
}

BENCHMARK_F(Delete, Zip, YggZipTreeSearchFixture<BasicTreeOptions>, 1000, 1)
{
  for (auto & v : this->search_values) {
    this->t.remove(*v);
  }

  celero::DoNotOptimizeAway(this->t);
}

BENCHMARK_F(Delete, NoStoreZip, YggZipTreeSearchFixture<ZTreeNoStoreOptions>,
            1000, 1)
{
  for (auto & v : this->search_values) {
    this->t.remove(*v);
  }

  celero::DoNotOptimizeAway(this->t);
}

BENCHMARK_F(Delete, StdSet, StdSetSearchFixture, 1000, 1)
{
  for (auto & v : this->search_values) {
    this->nodes.push_back(this->s.extract(v));
  }

  celero::DoNotOptimizeAway(this->s);
  celero::DoNotOptimizeAway(this->nodes);
}

BENCHMARK_F(Delete, BoostSet, BoostSetSearchFixture, 1000, 1)
{
  for (auto & v : this->search_values) {
    this->t.erase(*v);
  }

  celero::DoNotOptimizeAway(this->t);
}

BENCHMARK_F(Delete, MultiYgg, YggRBTreeSearchFixture<MultiTreeOptions>, 1000, 1)
{
  for (auto & v : this->search_values) {
    this->t.remove(*v);
  }

  celero::DoNotOptimizeAway(this->t);
}

BENCHMARK_F(Delete, CompressedYgg,
            YggRBTreeSearchFixture<CompressedTreeOptions>, 1000, 1)
{
  for (auto & v : this->search_values) {
    this->t.remove(*v);
  }

  celero::DoNotOptimizeAway(this->t);
}

BENCHMARK_F(Delete, BoostMultiSet, BoostMultiSetSearchFixture, 1000, 1)
{
  for (auto & v : this->search_values) {
    this->t.erase(*v);
  }

  celero::DoNotOptimizeAway(this->t);
}

#endif // YGG_BENCH_RBTREE_HPP
