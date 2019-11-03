#ifndef BENCH_BST_SEARCH_HPP
#define BENCH_BST_SEARCH_HPP

#include "common_bst.hpp"

struct BSTSearchOptions : public DefaultBenchmarkOptions
{
	using MainRandomizer = UseUniform;
	constexpr static bool need_values = true;
	using ValueRandomizer = DYN_GENERATOR;
	constexpr static bool values_from_fixed = true; // TODO take a percentage?
};

// TODO search for values not in the tree?

/*
 * Ygg's Red-Black Tree, using arithmetics instead of conditionals
 */
using SearchYggRBBSTFixtureArith =
    BSTFixture<YggRBTreeInterface<ArithTreeOptions>, SearchExperiment,
               BSTSearchOptions>;
BENCHMARK_DEFINE_F(SearchYggRBBSTFixtureArith, BM_BST_Search)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto val : this->experiment_values) {
			auto node = this->t.find(val);
			benchmark::DoNotOptimize(node);
		}
		this->papi.stop();
		// TODO shuffling?
	}

	this->papi.report_and_reset(state);
}
REGISTER(SearchYggRBBSTFixtureArith, BM_BST_Search)

/*
 * Ygg's Red-Black Tree
 */
using SearchYggRBBSTFixture = BSTFixture<YggRBTreeInterface<BasicTreeOptions>,
                                         SearchExperiment, BSTSearchOptions>;
BENCHMARK_DEFINE_F(SearchYggRBBSTFixture, BM_BST_Search)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto val : this->experiment_values) {
			auto node = this->t.find(val);
			benchmark::DoNotOptimize(node);
		}
		this->papi.stop();
		// TODO shuffling?
	}

	this->papi.report_and_reset(state);
}
REGISTER(SearchYggRBBSTFixture, BM_BST_Search)

/*
 * Ygg's weight-balanced tree, default parameters, two-pass
 */
using SearchYggWBDefTPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTTwopassTreeOptions>, SearchExperiment,
               BSTSearchOptions>;
BENCHMARK_DEFINE_F(SearchYggWBDefTPBSTFixture, BM_BST_Search)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto val : this->experiment_values) {
			auto node = this->t.find(val);
			benchmark::DoNotOptimize(node);
		}
		this->papi.stop();
		// TODO shuffling?
	}

	this->papi.report_and_reset(state);
}
REGISTER(SearchYggWBDefTPBSTFixture, BM_BST_Search)

/*
 * Ygg's weight-balanced tree, default parameters, single-pass
 */
using SearchYggWBDefSPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTSinglepassTreeOptions>, SearchExperiment,
               BSTSearchOptions>;
BENCHMARK_DEFINE_F(SearchYggWBDefSPBSTFixture, BM_BST_Search)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto val : this->experiment_values) {
			auto node = this->t.find(val);
			benchmark::DoNotOptimize(node);
		}
		this->papi.stop();
		// TODO shuffling?
	}

	this->papi.report_and_reset(state);
}
REGISTER(SearchYggWBDefSPBSTFixture, BM_BST_Search)

/*
 * Ygg's weight-balanced tree, Lai / Wood
 */
using SearchYggWBLWSPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTSinglepassTreeOptions>, SearchExperiment,
               BSTSearchOptions>;
BENCHMARK_DEFINE_F(SearchYggWBLWSPBSTFixture, BM_BST_Search)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto val : this->experiment_values) {
			auto node = this->t.find(val);
			benchmark::DoNotOptimize(node);
		}
		this->papi.stop();
		// TODO shuffling?
	}

	this->papi.report_and_reset(state);
}
REGISTER(SearchYggWBLWSPBSTFixture, BM_BST_Search)

/*
 * Ygg's weight-balanced tree, Balanced
 */
using SearchYggWBBalSPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTSinglepassTreeOptions>, SearchExperiment,
               BSTSearchOptions>;
BENCHMARK_DEFINE_F(SearchYggWBBalSPBSTFixture, BM_BST_Search)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto val : this->experiment_values) {
			auto node = this->t.find(val);
			benchmark::DoNotOptimize(node);
		}
		this->papi.stop();
		// TODO shuffling?
	}

	this->papi.report_and_reset(state);
}
REGISTER(SearchYggWBBalSPBSTFixture, BM_BST_Search)

/*
 * Ygg's weight-balanced tree, Super-Balanced
 */
using SearchYggWBSuperBalSPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTSinglepassTreeOptions>, SearchExperiment,
               BSTSearchOptions>;
BENCHMARK_DEFINE_F(SearchYggWBSuperBalSPBSTFixture, BM_BST_Search)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto val : this->experiment_values) {
			auto node = this->t.find(val);
			benchmark::DoNotOptimize(node);
		}
		this->papi.stop();
		// TODO shuffling?
	}

	this->papi.report_and_reset(state);
}
REGISTER(SearchYggWBSuperBalSPBSTFixture, BM_BST_Search)

/*
 * Ygg's weight-balanced tree, Integral, SP
 */
using SearchYggWB32SPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTSinglepassTreeOptions>, SearchExperiment,
               BSTSearchOptions>;
BENCHMARK_DEFINE_F(SearchYggWB32SPBSTFixture, BM_BST_Search)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto val : this->experiment_values) {
			auto node = this->t.find(val);
			benchmark::DoNotOptimize(node);
		}
		this->papi.stop();
		// TODO shuffling?
	}

	this->papi.report_and_reset(state);
}
REGISTER(SearchYggWB32SPBSTFixture, BM_BST_Search)

/*
 * Ygg's Zip Tree
 */
using SearchYggZBSTFixture = BSTFixture<YggZTreeInterface<BasicTreeOptions>,
                                        SearchExperiment, BSTSearchOptions>;
BENCHMARK_DEFINE_F(SearchYggZBSTFixture, BM_BST_Search)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto val : this->experiment_values) {
			auto node = this->t.find(val);
			benchmark::DoNotOptimize(node);
		}
		this->papi.stop();
	}
	this->papi.report_and_reset(state);
}
REGISTER(SearchYggZBSTFixture, BM_BST_Search)

/*
 * Boost::Intrusive::Set
 */
using SearchBISetBSTFixture =
    BSTFixture<BoostSetInterface, SearchExperiment, BSTSearchOptions>;
BENCHMARK_DEFINE_F(SearchBISetBSTFixture, BM_BST_Search)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto val : this->experiment_values) {
			auto node = this->t.find(val);
			benchmark::DoNotOptimize(node);
		}
		this->papi.stop();
	}
	this->papi.report_and_reset(state);
}
REGISTER(SearchBISetBSTFixture, BM_BST_Search)

/*
 * std::set
 */
using SearchStdSetBSTFixture =
    BSTFixture<StdSetInterface, SearchExperiment, BSTSearchOptions>;
BENCHMARK_DEFINE_F(SearchStdSetBSTFixture, BM_BST_Search)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto val : this->experiment_values) {
			auto node = this->t.find(val);
			benchmark::DoNotOptimize(node);
		}
		this->papi.stop();
	}
	this->papi.report_and_reset(state);
}
REGISTER(SearchStdSetBSTFixture, BM_BST_Search)

#ifndef NOMAIN
#include "main.hpp"
#endif

#endif
