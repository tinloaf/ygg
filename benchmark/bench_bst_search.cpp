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
REGISTER(SearchYggRBBSTFixture, BM_BST_Search);

/*
 * Ygg's Zip Tree
 */
using SearchYggZBSTFixture =
	BSTFixture<YggZTreeInterface<BasicTreeOptions>, false, true, false, true>;
BENCHMARK_DEFINE_F(SearchYggZBSTFixture, BM_BST_Search)(benchmark::State & state)
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
REGISTER(SearchYggZBSTFixture, BM_BST_Search);

/*
 * Boost::Intrusive::Set
 */
using SearchBISetBSTFixture =
	BSTFixture<BoostSetInterface, false, true, false, true>;
BENCHMARK_DEFINE_F(SearchBISetBSTFixture, BM_BST_Search)(benchmark::State & state)
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
REGISTER(SearchBISetBSTFixture, BM_BST_Search);

/*
 * std::set
 */
using SearchStdSetBSTFixture =
	BSTFixture<StdSetInterface, false, true, false, true>;
BENCHMARK_DEFINE_F(SearchStdSetBSTFixture, BM_BST_Search)(benchmark::State & state)
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
REGISTER(SearchStdSetBSTFixture, BM_BST_Search);

#ifndef NOMAIN
#include "main.hpp"
#endif

#endif
