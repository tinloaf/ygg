#ifndef BENCH_BST_DELETE_HPP
#define BENCH_BST_DELETE_HPP

#include "common_bst.hpp"

#include <sstream>
#include <type_traits>

struct BSTDeleteOptions : public DefaultBenchmarkOptions
{
	using MainRandomizer = UseUniform;
	constexpr static bool need_node_pointers = true;
	using NodePointerRandomizer = UseUniform;

	constexpr static bool distinct = true;
	constexpr static bool values_from_fixed = true; // TODO this makes no sense!

#ifdef PRESORT
	constexpr static bool nodes_presort = true;
	constexpr static double nodes_presort_fraction = 0.5;
	constexpr static bool pointers_presort = true;
	constexpr static double pointers_presort_fraction = 0.5;
#endif
};

/*
 * Ygg's Red-Black Tree
 */
using DeleteYggRBBSTFixture = BSTFixture<YggRBTreeInterface<BasicTreeOptions>,
                                         DeleteExperiment, BSTDeleteOptions>;
BENCHMARK_DEFINE_F(DeleteYggRBBSTFixture, BM_BST_Deletion)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			this->t.remove(*n);
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
REGISTER(DeleteYggRBBSTFixture, BM_BST_Deletion)

/*
 * Ygg's Red-Black Tree, avoiding conditional branches
 */
using DeleteYggRBBSTFixtureArith =
    BSTFixture<YggRBTreeInterface<ArithTreeOptions, RBBSTNamerArith>,
               DeleteExperiment, BSTDeleteOptions>;
BENCHMARK_DEFINE_F(DeleteYggRBBSTFixtureArith, BM_BST_Deletion)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			this->t.remove(*n);
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
REGISTER(DeleteYggRBBSTFixtureArith, BM_BST_Deletion)

/*
 * Ygg's Weight-Balanced Trees
 */
// Default gamma, delta / twopass
using DeleteYggWBDefGDefDTPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTTwopassTreeOptions, WBBSTNamerDefGDefDTP>,
               DeleteExperiment, BSTDeleteOptions>;
BENCHMARK_DEFINE_F(DeleteYggWBDefGDefDTPBSTFixture, BM_BST_Deletion)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			this->t.remove(*n);
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
REGISTER(DeleteYggWBDefGDefDTPBSTFixture, BM_BST_Deletion)

// Default gamma, delta / single pass
using DeleteYggWBDefGDefDSPBSTFixture = BSTFixture<
    YggWBTreeInterface<WBTSinglepassTreeOptions, WBBSTNamerDefGDefDSP>,
    DeleteExperiment, BSTDeleteOptions>;
BENCHMARK_DEFINE_F(DeleteYggWBDefGDefDSPBSTFixture, BM_BST_Deletion)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			this->t.remove(*n);
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
REGISTER(DeleteYggWBDefGDefDSPBSTFixture, BM_BST_Deletion)

// integral gamma, delta / single pass
using DeleteYggWB3G2DSPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTSinglepass32TreeOptions, WBBSTNamer3G2DSP>,
               DeleteExperiment, BSTDeleteOptions>;
BENCHMARK_DEFINE_F(DeleteYggWB3G2DSPBSTFixture, BM_BST_Deletion)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			this->t.remove(*n);
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
REGISTER(DeleteYggWB3G2DSPBSTFixture, BM_BST_Deletion)

// integral gamma, delta / twopass
using DeleteYggWB3G2DTPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTTwopass32TreeOptions, WBBSTNamer3G2DTP>,
               DeleteExperiment, BSTDeleteOptions>;
BENCHMARK_DEFINE_F(DeleteYggWB3G2DTPBSTFixture, BM_BST_Deletion)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			this->t.remove(*n);
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
REGISTER(DeleteYggWB3G2DTPBSTFixture, BM_BST_Deletion)

// Lai / Wood
using DeleteYggWBLWSPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTSinglepassLWTreeOptions, WBBSTNamerLWSP>,
               DeleteExperiment, BSTDeleteOptions>;
BENCHMARK_DEFINE_F(DeleteYggWBLWSPBSTFixture, BM_BST_Deletion)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			this->t.remove(*n);
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
REGISTER(DeleteYggWBLWSPBSTFixture, BM_BST_Deletion)

// Balanced
using DeleteYggWBBalSPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTSinglepassBalTreeOptions, WBBSTNamerBalSP>,
               DeleteExperiment, BSTDeleteOptions>;
BENCHMARK_DEFINE_F(DeleteYggWBBalSPBSTFixture, BM_BST_Deletion)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			this->t.remove(*n);
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
REGISTER(DeleteYggWBBalSPBSTFixture, BM_BST_Deletion)

// Super-Balanced
using DeleteYggWBSuperBalSPBSTFixture = BSTFixture<
    YggWBTreeInterface<WBTSinglepassSuperBalTreeOptions, WBBSTNamerSuperBalSP>,
    DeleteExperiment, BSTDeleteOptions>;
BENCHMARK_DEFINE_F(DeleteYggWBSuperBalSPBSTFixture, BM_BST_Deletion)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			this->t.remove(*n);
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
REGISTER(DeleteYggWBSuperBalSPBSTFixture, BM_BST_Deletion)

/*
 * Ygg's Energy-Balanced Tree
 */
using DeleteYggEBSTFixture =
    BSTFixture<YggEnergyTreeInterface<BasicTreeOptions>, DeleteExperiment,
               BSTDeleteOptions>;
BENCHMARK_DEFINE_F(DeleteYggEBSTFixture, BM_BST_Deletion)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			this->t.remove(*n);
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
REGISTER(DeleteYggEBSTFixture, BM_BST_Deletion)

/*
 * Ygg's Zip Tree
 */
using DeleteYggZBSTFixture = BSTFixture<YggZTreeInterface<BasicTreeOptions>,
                                        DeleteExperiment, BSTDeleteOptions>;
BENCHMARK_DEFINE_F(DeleteYggZBSTFixture, BM_BST_Deletion)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto n : this->experiment_node_pointers) {
			this->t.remove(*n);
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
REGISTER(DeleteYggZBSTFixture, BM_BST_Deletion)

/*
 * Boost::Intrusive::Set
 */
using DeleteBISetBSTFixture =
    BSTFixture<BoostSetInterface, DeleteExperiment, BSTDeleteOptions>;
BENCHMARK_DEFINE_F(DeleteBISetBSTFixture, BM_BST_Deletion)
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
REGISTER(DeleteBISetBSTFixture, BM_BST_Deletion)

/*
 * std::set
 */
using DeleteStdSetBSTFixture =
    BSTFixture<StdSetInterface, DeleteExperiment, BSTDeleteOptions>;
BENCHMARK_DEFINE_F(DeleteStdSetBSTFixture, BM_BST_Deletion)
(benchmark::State & state)
{
	std::vector<decltype(this->t)::const_iterator> experiment_iterators;
	std::vector<decltype(std::multiset<int>().extract(0))> extracted_nodes;
	extracted_nodes.reserve(static_cast<size_t>(state.range(1)));
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
REGISTER(DeleteStdSetBSTFixture, BM_BST_Deletion)

#ifndef NOMAIN
#include "main.hpp"
#endif

#endif
