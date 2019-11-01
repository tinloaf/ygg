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

/*
 * WBT-DST with 3/2
 */
using Insert32WBDSTFixture = DSTFixture<
    WBDSTInterface<BasicDSTTreeOptions, ygg::TreeFlags::WBT_DELTA_NUMERATOR<3>,
                   ygg::TreeFlags::WBT_DELTA_DENOMINATOR<1>,
                   ygg::TreeFlags::WBT_GAMMA_NUMERATOR<2>,
                   ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<1>>,
    InsertExperiment, true, false, false, false>;
BENCHMARK_DEFINE_F(Insert32WBDSTFixture, BM_DST_Insertion)
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
REGISTER(Insert32WBDSTFixture, BM_DST_Insertion)

#ifndef NOMAIN
#include "main.hpp"
#endif

#endif
