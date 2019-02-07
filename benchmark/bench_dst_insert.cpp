#ifndef BENCH_DST_INSERT_HPP
#define BENCH_DST_INSERT_HPP

#include "common_dst.hpp"

/*
 * Red-Black DST
 */
using InsertRBDSTFixture =
	DSTFixture<RBDSTInterface<BasicDSTTreeOptions>, true, false, false, false>;
BENCHMARK_DEFINE_F(InsertRBDSTFixture, BM_DST_Insertion)(benchmark::State & state)
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
BENCHMARK_REGISTER_F(InsertRBDSTFixture, BM_DST_Insertion)
    ->RangeMultiplier(2)
    ->Ranges({{BASE_SIZE, BASE_SIZE * (1 << DOUBLINGS)}, {EXPERIMENT_SIZE, EXPERIMENT_SIZE}});

/*
 * Zip DST
 */
using InsertZDSTFixture =
	DSTFixture<ZDSTInterface<BasicDSTTreeOptions>, true, false, false, false>;
BENCHMARK_DEFINE_F(InsertZDSTFixture, BM_DST_Insertion)(benchmark::State & state)
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
BENCHMARK_REGISTER_F(InsertZDSTFixture, BM_DST_Insertion)
    ->RangeMultiplier(2)
    ->Ranges({{BASE_SIZE, BASE_SIZE * (1 << DOUBLINGS)}, {EXPERIMENT_SIZE, EXPERIMENT_SIZE}});

#endif
