#ifndef BENCH_DST_INSERT_HPP
#define BENCH_DST_INSERT_HPP

#include "common_dst.hpp"

/*
 * Red-Black DST
 */
using InsertRBDSTFixture =
    DSTFixture<RBDSTInterface<BasicDSTTreeOptions>, InsertExperiment, true,
               false, false, false>;
BENCHMARK_DEFINE_F(InsertRBDSTFixture, BM_DST_Insertion)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto & n : this->experiment_nodes) {
			this->t.insert(n);
		}
		this->papi.stop();

		state.PauseTiming();
		for (auto & n : this->experiment_nodes) {
			this->t.remove(n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(InsertRBDSTFixture, BM_DST_Insertion)

/*
 * Zip DST
 */
using InsertZDSTFixture =
    DSTFixture<ZDSTInterface<BasicDSTTreeOptions>, InsertExperiment, true,
               false, false, false>;
BENCHMARK_DEFINE_F(InsertZDSTFixture, BM_DST_Insertion)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto & n : this->experiment_nodes) {
			this->t.insert(n);
		}
		this->papi.stop();

		state.PauseTiming();
		for (auto & n : this->experiment_nodes) {
			this->t.remove(n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(InsertZDSTFixture, BM_DST_Insertion)

#ifndef NOMAIN
#include "main.hpp"
#endif

#endif
