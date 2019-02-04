#ifndef BENCH_BST_INSERT_HPP
#define BENCH_BST_INSERT_HPP

#include "common_bst.hpp"

/*
 * Ygg's Red-Black Tree
 */
using YggRBFixture =
	Fixture<YggRBTreeInterface<BasicTreeOptions>, true, false, false, false>;
BENCHMARK_DEFINE_F(YggRBFixture, BM_Insertion)(benchmark::State & state)
{
	for (auto _ : state) {
		for (auto & n : this->experiment_nodes) {
			this->t.insert(n);
		}

		state.PauseTiming();
		for (auto & n : this->experiment_nodes) {
			this->t.remove(n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}
}
BENCHMARK_REGISTER_F(YggRBFixture, BM_Insertion)->Args({1000, 1000});

/*
 * Ygg's Zip Tree
 */
using YggZFixture =
	Fixture<YggZTreeInterface<BasicTreeOptions>, true, false, false, false>;
BENCHMARK_DEFINE_F(YggZFixture, BM_Insertion)(benchmark::State & state)
{
	for (auto _ : state) {
		for (auto & n : this->experiment_nodes) {
			this->t.insert(n);
		}

		state.PauseTiming();
		for (auto & n : this->experiment_nodes) {
			this->t.remove(n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}
}
BENCHMARK_REGISTER_F(YggZFixture, BM_Insertion)->Args({1000, 1000});

/*
 * Boost::Intrusive::Set
 */
using BISetFixture =
	Fixture<BoostSetInterface, true, false, false, false>;
BENCHMARK_DEFINE_F(BISetFixture, BM_Insertion)(benchmark::State & state)
{
	for (auto _ : state) {
		for (auto & n : this->experiment_nodes) {
			this->t.insert(n);
		}

		state.PauseTiming();
		for (auto & n : this->experiment_nodes) {
			this->t.erase(n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}
}
BENCHMARK_REGISTER_F(BISetFixture, BM_Insertion)->Args({1000, 1000});


BENCHMARK_MAIN();

#endif
