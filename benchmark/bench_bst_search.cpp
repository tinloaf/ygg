#ifndef BENCH_BST_SEARCH_HPP
#define BENCH_BST_SEARCH_HPP

#include "common_bst.hpp"

// TODO search for values not in the tree?

/*
 * Ygg's Red-Black Tree
 */
using SearchYggRBBSTFixture =
	BSTFixture<YggRBTreeInterface<BasicTreeOptions>, false, true, false, true>;
BENCHMARK_DEFINE_F(SearchYggRBBSTFixture, BM_BST_Search)(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi_start();
		for (auto val : this->experiment_values) {
			auto node = this->t.find(val);
			benchmark::DoNotOptimize(node);
		}
		this->papi_stop();
		// TODO shuffling?
	}

	this->papi_report_and_reset(state);
}
BENCHMARK_REGISTER_F(SearchYggRBBSTFixture, BM_BST_Search)
    ->RangeMultiplier(2)
    ->Ranges({{BASE_SIZE, BASE_SIZE * (1 << DOUBLINGS)}, {EXPERIMENT_SIZE, EXPERIMENT_SIZE}});

/*
 * Ygg's Zip Tree
 */
using SearchYggZBSTFixture =
	BSTFixture<YggZTreeInterface<BasicTreeOptions>, false, true, false, true>;
BENCHMARK_DEFINE_F(SearchYggZBSTFixture, BM_BST_Search)(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi_start();
		for (auto val : this->experiment_values) {
			auto node = this->t.find(val);
			benchmark::DoNotOptimize(node);
		}
		this->papi_stop();
	}
	this->papi_report_and_reset(state);
}
BENCHMARK_REGISTER_F(SearchYggZBSTFixture, BM_BST_Search)
    ->RangeMultiplier(2)
    ->Ranges({{BASE_SIZE, BASE_SIZE * (1 << DOUBLINGS)}, {EXPERIMENT_SIZE, EXPERIMENT_SIZE}});

/*
 * Boost::Intrusive::Set
 */
using SearchBISetBSTFixture =
	BSTFixture<BoostSetInterface, false, true, false, true>;
BENCHMARK_DEFINE_F(SearchBISetBSTFixture, BM_BST_Search)(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi_start();
		for (auto val : this->experiment_values) {
			auto node = this->t.find(val);
			benchmark::DoNotOptimize(node);
		}
		this->papi_stop();
	}
	this->papi_report_and_reset(state);
}
BENCHMARK_REGISTER_F(SearchBISetBSTFixture, BM_BST_Search)
    ->RangeMultiplier(2)
    ->Ranges({{BASE_SIZE, BASE_SIZE * (1 << DOUBLINGS)}, {EXPERIMENT_SIZE, EXPERIMENT_SIZE}});

/*
 * std::set
 */
using SearchStdSetBSTFixture =
	BSTFixture<StdSetInterface, false, true, false, true>;
BENCHMARK_DEFINE_F(SearchStdSetBSTFixture, BM_BST_Search)(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi_start();
		for (auto val : this->experiment_values) {
			auto node = this->t.find(val);
			benchmark::DoNotOptimize(node);
		}
		this->papi_stop();
	}
	this->papi_report_and_reset(state);
}
BENCHMARK_REGISTER_F(SearchStdSetBSTFixture, BM_BST_Search)
    ->RangeMultiplier(2)
    ->Ranges({{BASE_SIZE, BASE_SIZE * (1 << DOUBLINGS)}, {EXPERIMENT_SIZE, EXPERIMENT_SIZE}});

#endif
