#ifndef BENCH_DST_DELETE_HPP
#define BENCH_DST_DELETE_HPP

#include "common_dst.hpp"

/*
 * Red-Black DST
 */
using DeleteRBDSTFixture =
	DSTFixture<RBDSTInterface<BasicDSTTreeOptions>, false, false, true, false>;
BENCHMARK_DEFINE_F(DeleteRBDSTFixture, BM_DST_Deletion)(benchmark::State & state)
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
BENCHMARK_REGISTER_F(DeleteRBDSTFixture, BM_DST_Deletion)
    ->RangeMultiplier(2)
    ->Ranges({{BASE_SIZE, BASE_SIZE * (1 << DOUBLINGS)}, {EXPERIMENT_SIZE, EXPERIMENT_SIZE}});

/*
 * Zip DST
 */
using DeleteZDSTFixture =
	DSTFixture<ZDSTInterface<BasicDSTTreeOptions>, false, false, true, false>;
BENCHMARK_DEFINE_F(DeleteZDSTFixture, BM_DST_Deletion)(benchmark::State & state)
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
BENCHMARK_REGISTER_F(DeleteZDSTFixture, BM_DST_Deletion)
    ->RangeMultiplier(2)
    ->Ranges({{BASE_SIZE, BASE_SIZE * (1 << DOUBLINGS)}, {EXPERIMENT_SIZE, EXPERIMENT_SIZE}});

#endif
