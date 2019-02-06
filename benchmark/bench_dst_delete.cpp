#ifndef BENCH_DST_INSERT_HPP
#define BENCH_DST_INSERT_HPP

#include "common_dst.hpp"

/*
 * Red-Black DST
 */
using RBDSTFixture =
	Fixture<RBDSTInterface<BasicTreeOptions>, false, false, true, false>;
BENCHMARK_DEFINE_F(RBDSTFixture, BM_Deletion)(benchmark::State & state)
{
	for (auto _ : state) {
		for (auto i : this->experiment_indices) {
			this->t.remove(this->fixed_nodes[i]);
		}

		state.PauseTiming();
		for (auto i : this->experiment_indices) {
			this->t.insert(this->fixed_nodes[i]);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}
}
BENCHMARK_REGISTER_F(RBDSTFixture, BM_Deletion)->Args({1000, 1000});

/*
 * Zip DST
 */
using ZDSTFixture =
	Fixture<ZDSTInterface<BasicTreeOptions>, false, false, true, false>;
BENCHMARK_DEFINE_F(ZDSTFixture, BM_Deletion)(benchmark::State & state)
{
	for (auto _ : state) {
		for (auto i : this->experiment_indices) {
			this->t.remove(this->fixed_nodes[i]);
		}

		state.PauseTiming();
		for (auto i : this->experiment_indices) {
			this->t.insert(this->fixed_nodes[i]);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}
}
BENCHMARK_REGISTER_F(ZDSTFixture, BM_Deletion)->Args({1000, 1000});


BENCHMARK_MAIN();

#endif
