#ifndef BENCH_BST_INSERT_HPP
#define BENCH_BST_INSERT_HPP

#include "common_bst.hpp"

// TODO search for values not in the tree?

/*
 * Ygg's Red-Black Tree
 */
using YggRBFixture =
	Fixture<YggRBTreeInterface<BasicTreeOptions>, false, true, false, true>;
BENCHMARK_DEFINE_F(YggRBFixture, BM_Search)(benchmark::State & state)
{
	for (auto _ : state) {
		for (auto val : this->experiment_values) {
			auto node = this->t.find(val);
			benchmark::DoNotOptimize(node);
		}

		// TODO shuffling?
	}
}
BENCHMARK_REGISTER_F(YggRBFixture, BM_Search)->Args({1000, 100});

/*
 * Ygg's Zip Tree
 */
using YggZFixture =
	Fixture<YggZTreeInterface<BasicTreeOptions>, false, true, false, true>;
BENCHMARK_DEFINE_F(YggZFixture, BM_Search)(benchmark::State & state)
{
	for (auto _ : state) {
		for (auto val : this->experiment_values) {
			auto node = this->t.find(val);
			benchmark::DoNotOptimize(node);
		}
	}
}
BENCHMARK_REGISTER_F(YggZFixture, BM_Search)->Args({1000, 100});

/*
 * Boost::Intrusive::Set
 */
using BISetFixture =
	Fixture<BoostSetInterface, false, true, false, true>;
BENCHMARK_DEFINE_F(BISetFixture, BM_Search)(benchmark::State & state)
{
	for (auto _ : state) {
		for (auto val : this->experiment_values) {
			auto node = this->t.find(val);
			benchmark::DoNotOptimize(node);
		}
	}
}
BENCHMARK_REGISTER_F(BISetFixture, BM_Search)->Args({1000, 100});


BENCHMARK_MAIN();

#endif
