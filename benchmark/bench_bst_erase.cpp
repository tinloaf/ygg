#ifndef BENCH_BST_ERASE_HPP
#define BENCH_BST_ERASE_HPP

#include "common_bst.hpp"

struct BSTEraseOptions : public DefaultBenchmarkOptions
{
	using MainRandomizer = DYN_GENERATOR;
	constexpr static bool need_node_pointers = true;
	using NodePointerRandomizer = UseUniform;

	//	constexpr static bool distinct = true;
	constexpr static bool distinct = false; // Distinctness in values and the Zipf
	                                        // generator don't work together
	constexpr static bool node_pointers_distinct =
	    true; // But we can only delete every node once.

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
	std::vector<typename decltype(this->experiment_node_pointers)::value_type>
	    erased_nodes;
	erased_nodes.reserve(this->experiment_node_pointers.size());

	Clock c;
	for (auto _ : state) {
		c.start();
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			erased_nodes.push_back(this->t.erase(n->get_value()));
		}
		this->papi.stop();
		state.SetIterationTime(c.get());

		for (auto n : erased_nodes) {
			//		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		erased_nodes.clear();
		// TODO shuffling here?
	}

	this->papi.report_and_reset(state);
}
REGISTER(EraseYggRBBSTFixture, BM_BST_Erasure)

/*
 * Ygg's Red-Black Tree, avoiding conditional branches
 */
using EraseYggRBBSTFixtureArith =
    BSTFixture<YggRBTreeInterface<ArithTreeOptions>, EraseExperiment,
               BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggRBBSTFixtureArith, BM_BST_Erasure)
(benchmark::State & state)
{
	std::vector<typename decltype(this->experiment_node_pointers)::value_type>
	    erased_nodes;
	erased_nodes.reserve(this->experiment_node_pointers.size());

	Clock c;
	for (auto _ : state) {
		c.start();
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			erased_nodes.push_back(this->t.erase(n->get_value()));
		}
		this->papi.stop();
		state.SetIterationTime(c.get());

		for (auto n : erased_nodes) {
			//		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		erased_nodes.clear();
		// TODO shuffling here?
	}

	this->papi.report_and_reset(state);
}
REGISTER(EraseYggRBBSTFixtureArith, BM_BST_Erasure)

/*
 * Ygg's Red-Black Tree, with prefetching
 */
using EraseYggRBBSTFixturePF =
    BSTFixture<YggRBTreeInterface<RBPrefetchTreeOptions>, EraseExperiment,
               BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggRBBSTFixturePF, BM_BST_Erasure)
(benchmark::State & state)
{
	std::vector<typename decltype(this->experiment_node_pointers)::value_type>
	    erased_nodes;
	erased_nodes.reserve(this->experiment_node_pointers.size());

	Clock c;
	for (auto _ : state) {
		c.start();
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			erased_nodes.push_back(this->t.erase(n->get_value()));
		}
		this->papi.stop();
		state.SetIterationTime(c.get());

		for (auto n : erased_nodes) {
			//		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		erased_nodes.clear();
		// TODO shuffling here?
	}

	this->papi.report_and_reset(state);
}
REGISTER(EraseYggRBBSTFixturePF, BM_BST_Erasure)

/*
 * Ygg's Weight-Balanced Trees
 */
// Default gamma, delta / twopass
using EraseYggWBDefGDefDTPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTTwopassTreeOptions>, EraseExperiment,
               BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggWBDefGDefDTPBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
	std::vector<typename decltype(this->experiment_node_pointers)::value_type>
	    erased_nodes;
	erased_nodes.reserve(this->experiment_node_pointers.size());

	Clock c;
	for (auto _ : state) {
		c.start();
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			erased_nodes.push_back(this->t.erase_optimistic(n->get_value()));
		}
		this->papi.stop();
		state.SetIterationTime(c.get());

		for (auto n : erased_nodes) {
			//		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		erased_nodes.clear();
		// TODO shuffling here?
	}

	this->papi.report_and_reset(state);
}
REGISTER(EraseYggWBDefGDefDTPBSTFixture, BM_BST_Erasure)

// Default gamma, delta / single pass
using EraseYggWBDefGDefDSPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTSinglepassTreeOptions>, EraseExperiment,
               BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggWBDefGDefDSPBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
	std::vector<typename decltype(this->experiment_node_pointers)::value_type>
	    erased_nodes;
	erased_nodes.reserve(this->experiment_node_pointers.size());

	Clock c;
	for (auto _ : state) {
		c.start();
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			erased_nodes.push_back(this->t.erase_optimistic(n->get_value()));
		}
		this->papi.stop();
		state.SetIterationTime(c.get());

		for (auto n : erased_nodes) {
			//		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		erased_nodes.clear();
		// TODO shuffling here?
	}

	this->papi.report_and_reset(state);
}
REGISTER(EraseYggWBDefGDefDSPBSTFixture, BM_BST_Erasure)

// Lai and Wood Gamma, Delta / single pass
using EraseYggWBLWSPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTSinglepassLWTreeOptions>, EraseExperiment,
               BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggWBLWSPBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
	std::vector<typename decltype(this->experiment_node_pointers)::value_type>
	    erased_nodes;
	erased_nodes.reserve(this->experiment_node_pointers.size());

	Clock c;
	for (auto _ : state) {
		c.start();
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			erased_nodes.push_back(this->t.erase_optimistic(n->get_value()));
		}
		this->papi.stop();
		state.SetIterationTime(c.get());

		for (auto n : erased_nodes) {
			//		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		erased_nodes.clear();
		// TODO shuffling here?
	}

	this->papi.report_and_reset(state);
}
REGISTER(EraseYggWBLWSPBSTFixture, BM_BST_Erasure)

// Balance-focussed Gamma, Delta / single pass
using EraseYggWBBalSPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTSinglepassBalTreeOptions>, EraseExperiment,
               BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggWBBalSPBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
	std::vector<typename decltype(this->experiment_node_pointers)::value_type>
	    erased_nodes;
	erased_nodes.reserve(this->experiment_node_pointers.size());

	Clock c;
	for (auto _ : state) {
		c.start();
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			erased_nodes.push_back(this->t.erase_optimistic(n->get_value()));
		}
		this->papi.stop();
		state.SetIterationTime(c.get());

		for (auto n : erased_nodes) {
			//		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		erased_nodes.clear();
		// TODO shuffling here?
	}

	this->papi.report_and_reset(state);
}
REGISTER(EraseYggWBBalSPBSTFixture, BM_BST_Erasure)

// Super-Balance-focussed Gamma, Delta / single pass
using EraseYggWBSuperBalSPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTSinglepassSuperBalTreeOptions>,
               EraseExperiment, BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggWBSuperBalSPBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
	std::vector<typename decltype(this->experiment_node_pointers)::value_type>
	    erased_nodes;
	erased_nodes.reserve(this->experiment_node_pointers.size());

	Clock c;
	for (auto _ : state) {
		c.start();
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			erased_nodes.push_back(this->t.erase_optimistic(n->get_value()));
		}
		this->papi.stop();
		state.SetIterationTime(c.get());

		for (auto n : erased_nodes) {
			//		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		erased_nodes.clear();
		// TODO shuffling here?
	}

	this->papi.report_and_reset(state);
}
REGISTER(EraseYggWBSuperBalSPBSTFixture, BM_BST_Erasure)

// Balance-focussed Gamma, Delta / single pass, avoiding conditionals
using EraseYggWBBalSPArithBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTSinglepassBalArithTreeOptions>,
               EraseExperiment, BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggWBBalSPArithBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
	std::vector<typename decltype(this->experiment_node_pointers)::value_type>
	    erased_nodes;
	erased_nodes.reserve(this->experiment_node_pointers.size());

	Clock c;
	for (auto _ : state) {
		c.start();
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			erased_nodes.push_back(this->t.erase_optimistic(n->get_value()));
		}
		this->papi.stop();
		state.SetIterationTime(c.get());

		for (auto n : erased_nodes) {
			//		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		erased_nodes.clear();
		// TODO shuffling here?
	}

	this->papi.report_and_reset(state);
}
REGISTER(EraseYggWBBalSPArithBSTFixture, BM_BST_Erasure)

// Default gamma, delta / single pass, optimistic
using EraseYggWBDefGDefDSPOPTBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTSinglepassTreeOptions>, EraseExperiment,
               BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggWBDefGDefDSPOPTBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
	std::vector<typename decltype(this->experiment_node_pointers)::value_type>
	    erased_nodes;
	erased_nodes.reserve(this->experiment_node_pointers.size());

	Clock c;
	for (auto _ : state) {
		c.start();
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			erased_nodes.push_back(this->t.erase_optimistic(n->get_value()));
		}
		this->papi.stop();
		state.SetIterationTime(c.get());

		for (auto n : erased_nodes) {
			//		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		erased_nodes.clear();
		// TODO shuffling here?
	}

	this->papi.report_and_reset(state);
}
REGISTER(EraseYggWBDefGDefDSPOPTBSTFixture, BM_BST_Erasure)

// integral gamma, delta / single pass
using EraseYggWB3G2DSPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTSinglepass32TreeOptions>, EraseExperiment,
               BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggWB3G2DSPBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
	std::vector<typename decltype(this->experiment_node_pointers)::value_type>
	    erased_nodes;
	erased_nodes.reserve(this->experiment_node_pointers.size());

	Clock c;
	for (auto _ : state) {
		c.start();
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			erased_nodes.push_back(this->t.erase_optimistic(n->get_value()));
		}
		this->papi.stop();
		state.SetIterationTime(c.get());

		for (auto n : erased_nodes) {
			//		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		erased_nodes.clear();
		// TODO shuffling here?
	}

	this->papi.report_and_reset(state);
}
REGISTER(EraseYggWB3G2DSPBSTFixture, BM_BST_Erasure)

/*
// integral gamma, delta / twopass
using EraseYggWB3G2DTPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTTwopass32TreeOptions>,
               EraseExperiment, BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggWB3G2DTPBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
  std::vector<typename decltype(this->experiment_node_pointers)::value_type>
      erased_nodes;
  erased_nodes.reserve(this->experiment_node_pointers.size());

  Clock c; for (auto _ : state) {
    c.start(); this->papi.start();
    for (auto n : this->experiment_node_pointers) {
      erased_nodes.push_back(this->t.erase(n->get_value()));
    }
    this->papi.stop(); state.SetIterationTime(c.get());


    for (auto n : erased_nodes) {
      //		for (auto n : this->experiment_node_pointers) {
      this->t.insert(*n);
    }
    erased_nodes.clear();
    // TODO shuffling here?

  }

  this->papi.report_and_reset(state);
}
REGISTER(EraseYggWB3G2DTPBSTFixture, BM_BST_Erasure)
*/

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
  std::vector<typename decltype(this->experiment_node_pointers)::value_type>
      erased_nodes;
  erased_nodes.reserve(this->experiment_node_pointers.size());

  Clock c; for (auto _ : state) {
    c.start(); this->papi.start();
    for (auto n : this->experiment_node_pointers) {
      erased_nodes.push_back(this->t.erase(n->get_value()));
    }
    this->papi.stop(); state.SetIterationTime(c.get());


    for (auto n : erased_nodes) {
      //		for (auto n : this->experiment_node_pointers) {
      this->t.insert(*n);
    }
    erased_nodes.clear();
    // TODO shuffling here?

  }

  this->papi.report_and_reset(state);
}
REGISTER(EraseYggEBSTFixture, BM_BST_Erasure);
*/

/*
 * Ygg's Zip Tree
 */
/*
using EraseYggZBSTFixture = BSTFixture<YggZTreeInterface<BasicTreeOptions>,
                                       EraseExperiment, BSTEraseOptions>;
BENCHMARK_DEFINE_F(EraseYggZBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
  std::vector<typename decltype(this->experiment_node_pointers)::value_type>
      erased_nodes;
  erased_nodes.reserve(this->experiment_node_pointers.size());

  Clock c; for (auto _ : state) {
    c.start(); this->papi.start();
    for (auto n : this->experiment_node_pointers) {
      erased_nodes.push_back(this->t.erase(n->get_value()));
    }
    this->papi.stop(); state.SetIterationTime(c.get());


    for (auto n : erased_nodes) {
      //		for (auto n : this->experiment_node_pointers) {
      this->t.insert(*n);
    }
    erased_nodes.clear();
    // TODO shuffling here?

  }

  this->papi.report_and_reset(state);
}
REGISTER(EraseYggZBSTFixture, BM_BST_Erasure)
*/

/*
 * Boost::Intrusive::Set
 */
/*
using EraseBISetBSTFixture =
    BSTFixture<BoostSetInterface, EraseExperiment, UseUniform, UseNone, UseNone,
UseUniform, false>; BENCHMARK_DEFINE_F(EraseBISetBSTFixture, BM_BST_Erasure)
(benchmark::State & state)
{
TODO Rebuild for the new erased_node system
  Clock c; for (auto _ : state) {
    c.start(); this->papi.start();
    for (auto n : this->experiment_node_pointers) {
      this->t.erase(*n);
    }
    this->papi.stop(); state.SetIterationTime(c.get());


    for (auto n : this->experiment_node_pointers) {
      this->t.insert(*n);
    }
    // TODO shuffling here?

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

  Clock c; for (auto _ : state) {
    c.start(); this->papi.start();
    for (auto inner_it : experiment_iterators) {
      extracted_nodes.push_back(this->t.extract(inner_it));
    }
    this->papi.stop(); state.SetIterationTime(c.get());


    experiment_iterators.clear();
    for (auto & n : extracted_nodes) {
      experiment_iterators.push_back(this->t.insert(std::move(n)));
    }
    extracted_nodes.clear();
    // TODO shuffling here?

  }

  this->papi.report_and_reset(state);
}
REGISTER(EraseStdSetBSTFixture, BM_BST_Erasure);
*/

#ifndef NOMAIN
#include "main.hpp"
#endif

#endif
