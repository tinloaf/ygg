#ifndef BENCH_DST_INSERT_HPP
#define BENCH_DST_INSERT_HPP

#include "common_dst.hpp"

/*
 * Red-Black DST
 */
using RBDSTFixture =
	Fixture<RBDSTInterface<BasicTreeOptions>, true, false, false, false>;
BENCHMARK_DEFINE_F(RBDSTFixture, BM_Insertion)(benchmark::State & state)
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
BENCHMARK_REGISTER_F(RBDSTFixture, BM_Insertion)->Args({1000, 1000});

/*
 * Zip DST
 */
using ZDSTFixture =
	Fixture<ZDSTInterface<BasicTreeOptions>, true, false, false, false>;
BENCHMARK_DEFINE_F(ZDSTFixture, BM_Insertion)(benchmark::State & state)
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
BENCHMARK_REGISTER_F(ZDSTFixture, BM_Insertion)->Args({1000, 1000});


BENCHMARK_MAIN();

#endif
