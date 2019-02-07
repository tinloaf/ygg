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
REGISTER(InsertRBDSTFixture, BM_DST_Insertion);

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
REGISTER(InsertZDSTFixture, BM_DST_Insertion);

#ifndef NOMAIN
#include "main.hpp"
#endif

#endif
