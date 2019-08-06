#ifndef BENCH_BST_ERASE_HPP
#define BENCH_BST_ERASE_HPP

#include "common_bst.hpp"

struct BSTEraseOptions : public DefaultBenchmarkOptions
{
	using MainRandomizer = UseUniform;
	constexpr static bool need_node_pointers = true;
	using NodePointerRandomizer = UseUniform;

	constexpr static bool distinct = true;

#ifdef PRESORT
	constexpr static bool fixed_presort = true;
	constexpr static double fixed_presort_fraction = 0.5;
	constexpr static bool pointers_presort = true;
	constexpr static double pointers_presort_fraction = 0.5;
#endif
};

/*
 * Ygg's Red-Black Tree
 */
using EraseYggRBBSTFixture = BSTFixture<YggRBTreeInterface<BasicTreeOptions>,
                                        EraseExperiment, BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggRBBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			this->t.erase(n->get_value());
		}
		this->papi.stop();

		state.PauseTiming();
		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(EraseYggRBBSTFixture, BM_BST_Erasure)

/*
 * Ygg's Red-Black Tree, avoiding conditional branches
 */
using EraseYggRBBSTFixtureArith =
    BSTFixture<YggRBTreeInterface<ArithTreeOptions, RBBSTNamerArith>,
               EraseExperiment, BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggRBBSTFixtureArith, BM_BST_Erasure)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			this->t.erase(n->get_value());
		}
		this->papi.stop();

		state.PauseTiming();
		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(EraseYggRBBSTFixtureArith, BM_BST_Erasure)

/*
 * Ygg's Weight-Balanced Trees
 */
// Default gamma, delta / twopass
using EraseYggWBDefGDefDTPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTTwopassTreeOptions, WBBSTNamerDefGDefDTP>,
               EraseExperiment, BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggWBDefGDefDTPBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			this->t.erase(n->get_value());
		}
		this->papi.stop();

		state.PauseTiming();
		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(EraseYggWBDefGDefDTPBSTFixture, BM_BST_Erasure)

// Default gamma, delta / single pass
using EraseYggWBDefGDefDSPBSTFixture = BSTFixture<
    YggWBTreeInterface<WBTSinglepassTreeOptions, WBBSTNamerDefGDefDSP>,
    EraseExperiment, BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggWBDefGDefDSPBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			this->t.erase(n->get_value());
		}
		this->papi.stop();

		state.PauseTiming();
		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(EraseYggWBDefGDefDSPBSTFixture, BM_BST_Erasure)

// Lai and Wood Gamma, Delta / single pass
using EraseYggWBLWSPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTSinglepassLWTreeOptions, WBBSTNamerLWSP>,
               EraseExperiment, BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggWBLWSPBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			this->t.erase_optimistic(n->get_value());
		}
		this->papi.stop();

		state.PauseTiming();
		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(EraseYggWBLWSPBSTFixture, BM_BST_Erasure)

// Balance-focussed Gamma, Delta / single pass
using EraseYggWBBalSPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTSinglepassBalTreeOptions, WBBSTNamerBalSP>,
               EraseExperiment, BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggWBBalSPBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			this->t.erase_optimistic(n->get_value());
		}
		this->papi.stop();

		state.PauseTiming();
		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(EraseYggWBBalSPBSTFixture, BM_BST_Erasure)

// Super-Balance-focussed Gamma, Delta / single pass
using EraseYggWBSuperBalSPBSTFixture = BSTFixture<
    YggWBTreeInterface<WBTSinglepassSuperBalTreeOptions, WBBSTNamerSuperBalSP>,
    EraseExperiment, BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggWBSuperBalSPBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			this->t.erase_optimistic(n->get_value());
		}
		this->papi.stop();

		state.PauseTiming();
		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(EraseYggWBSuperBalSPBSTFixture, BM_BST_Erasure)

// Balance-focussed Gamma, Delta / single pass, avoiding conditionals
using EraseYggWBBalSPArithBSTFixture = BSTFixture<
    YggWBTreeInterface<WBTSinglepassBalArithTreeOptions, WBBSTNamerBalSPArith>,
    EraseExperiment, BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggWBBalSPArithBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			this->t.erase_optimistic(n->get_value());
		}
		this->papi.stop();

		state.PauseTiming();
		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(EraseYggWBBalSPArithBSTFixture, BM_BST_Erasure)

// Default gamma, delta / single pass, optimistic
using EraseYggWBDefGDefDSPOPTBSTFixture = BSTFixture<
    YggWBTreeInterface<WBTSinglepassTreeOptions, WBBSTNamerDefGDefDSPOPT>,
    EraseExperiment, BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggWBDefGDefDSPOPTBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			this->t.erase_optimistic(n->get_value());
		}
		this->papi.stop();

		state.PauseTiming();
		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(EraseYggWBDefGDefDSPOPTBSTFixture, BM_BST_Erasure)

// integral gamma, delta / single pass
using EraseYggWB3G2DSPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTSinglepass32TreeOptions, WBBSTNamer3G2DSP>,
               EraseExperiment, BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggWB3G2DSPBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
	for (auto _ : state) {
		(void)_;
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			this->t.erase(n->get_value());
		}
		this->papi.stop();

		state.PauseTiming();
		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(EraseYggWB3G2DSPBSTFixture, BM_BST_Erasure)

// integral gamma, delta / single pass (optimistic)
using EraseYggWB3G2DSPOPTBSTFixture = BSTFixture<
    YggWBTreeInterface<WBTSinglepass32TreeOptions, WBBSTNamer3G2DSPOPT>,
    EraseExperiment, BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggWB3G2DSPOPTBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			this->t.erase_optimistic(n->get_value());
		}
		this->papi.stop();

		state.PauseTiming();
		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(EraseYggWB3G2DSPOPTBSTFixture, BM_BST_Erasure)

// integral gamma, delta / twopass
using EraseYggWB3G2DTPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTTwopass32TreeOptions, WBBSTNamer3G2DTP>,
               EraseExperiment, BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggWB3G2DTPBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			this->t.erase(n->get_value());
		}
		this->papi.stop();

		state.PauseTiming();
		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(EraseYggWB3G2DTPBSTFixture, BM_BST_Erasure)

/*
 * Ygg's Energy-Balanced Tree
 */
/*
using EraseYggEBSTFixture =
    BSTFixture<YggEnergyTreeInterface<BasicTreeOptions>, EraseExperiment,
UseUniform, UseNone, UseNone, UseUniform, false>;
BENCHMARK_DEFINE_F(EraseYggEBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
  for (auto _ : state) {
    this->papi.start();
    for (auto n : this->experiment_node_pointers) {
      this->t.erase(n->get_value());
    }
    this->papi.stop();

    state.PauseTiming();
    for (auto n : this->experiment_node_pointers) {
      this->t.insert(*n);
    }
    // TODO shuffling here?
    state.ResumeTiming();
  }

  this->papi.report_and_reset(state);
}
REGISTER(EraseYggEBSTFixture, BM_BST_Erasure);
*/

/*
 * Ygg's Zip Tree
 */
using EraseYggZBSTFixture = BSTFixture<YggZTreeInterface<BasicTreeOptions>,
                                       EraseExperiment, BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggZBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			this->t.erase(n->get_value());
		}
		this->papi.stop();

		state.PauseTiming();
		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(EraseYggZBSTFixture, BM_BST_Erasure)

/*
 * Boost::Intrusive::Set
 */
/*
using EraseBISetBSTFixture =
    BSTFixture<BoostSetInterface, EraseExperiment, UseUniform, UseNone, UseNone,
UseUniform, false>; BENCHMARK_DEFINE_F(EraseBISetBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
  for (auto _ : state) {
    this->papi.start();
    for (auto n : this->experiment_node_pointers) {
      this->t.erase(*n);
    }
    this->papi.stop();

    state.PauseTiming();
    for (auto n : this->experiment_node_pointers) {
      this->t.insert(*n);
    }
    // TODO shuffling here?
    state.ResumeTiming();
  }

  this->papi.report_and_reset(state);
}
REGISTER(EraseBISetBSTFixture, BM_BST_Erasure);
*/
/*
 * std::set
 */
/*
using EraseStdSetBSTFixture =
    BSTFixture<StdSetInterface, EraseExperiment, UseUniform, UseNone, UseNone,
UseNone, false>; BENCHMARK_DEFINE_F(EraseStdSetBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
  std::vector<decltype(this->t)::const_iterator> experiment_iterators;
  std::vector<decltype(std::multiset<int>().extract(0))> extracted_nodes;
  extracted_nodes.reserve((size_t)state.range(1));
  std::vector<decltype(this->t)::const_iterator> all_iterators;
  auto it = this->t.begin();
  while (it != this->t.end()) {
    all_iterators.emplace_back(it);
    it++;
  }
  std::sample(all_iterators.begin(), all_iterators.end(),
              std::back_inserter(experiment_iterators), state.range(1),
              this->rng);
  all_iterators.clear();

  for (auto _ : state) {
    this->papi.start();
    for (auto inner_it : experiment_iterators) {
      extracted_nodes.push_back(this->t.extract(inner_it));
    }
    this->papi.stop();

    state.PauseTiming();
    experiment_iterators.clear();
    for (auto & n : extracted_nodes) {
      experiment_iterators.push_back(this->t.insert(std::move(n)));
    }
    extracted_nodes.clear();
    // TODO shuffling here?
    state.ResumeTiming();
  }

  this->papi.report_and_reset(state);
}
REGISTER(EraseStdSetBSTFixture, BM_BST_Erasure);
*/

#ifndef NOMAIN
#include "main.hpp"
#endif

#endif
