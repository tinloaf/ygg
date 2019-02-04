#ifndef BENCH_BST_INSERT_HPP
#define BENCH_BST_INSERT_HPP

#include "common_bst.hpp"

/*
 * Ygg's Red-Black Tree
 */
using YggRBFixture =
	Fixture<YggRBTreeInterface<BasicTreeOptions>, false, false, true, false>;
BENCHMARK_DEFINE_F(YggRBFixture, BM_Deletion)(benchmark::State & state)
{
	for (auto _ : state) {
		for (auto n : this->experiment_node_pointers) {
			this->t.remove(*n);
		}

		state.PauseTiming();
		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}
}
BENCHMARK_REGISTER_F(YggRBFixture, BM_Deletion)->Args({1000, 100});

/*
 * Ygg's Zip Tree
 */
using YggZFixture =
	Fixture<YggZTreeInterface<BasicTreeOptions>, false, false, true, false>;
BENCHMARK_DEFINE_F(YggZFixture, BM_Deletion)(benchmark::State & state)
{
	for (auto _ : state) {
		for (auto n : this->experiment_node_pointers) {
			this->t.remove(*n);
		}

		state.PauseTiming();
		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}
}
BENCHMARK_REGISTER_F(YggZFixture, BM_Deletion)->Args({1000, 100});

/*
 * Boost::Intrusive::Set
 */
using BISetFixture =
	Fixture<BoostSetInterface, false, false, true, false>;
BENCHMARK_DEFINE_F(BISetFixture, BM_Deletion)(benchmark::State & state)
{
	for (auto _ : state) {
		for (auto n : this->experiment_node_pointers) {
			this->t.erase(*n);
		}

		state.PauseTiming();
		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}
}
BENCHMARK_REGISTER_F(BISetFixture, BM_Deletion)->Args({1000, 100});


BENCHMARK_MAIN();

#endif
