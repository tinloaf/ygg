#ifndef BENCH_DST_DELETE_HPP
#define BENCH_DST_DELETE_HPP

#include "common_dst.hpp"

/*
 * Red-Black DST
 */
using DeleteRBDSTFixture =
    DSTFixture<RBDSTInterface<BasicDSTTreeOptions>, DeleteExperiment, false,
               false, true, false>;
BENCHMARK_DEFINE_F(DeleteRBDSTFixture, BM_DST_Deletion)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto i : this->experiment_indices) {
			this->t.remove(this->fixed_nodes[i]);
		}
		this->papi.stop();

		state.PauseTiming();
		for (auto i : this->experiment_indices) {
			this->t.insert(this->fixed_nodes[i]);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(DeleteRBDSTFixture, BM_DST_Deletion)

/*
 * Zip DST
 */
using DeleteZDSTFixture =
    DSTFixture<ZDSTInterface<BasicDSTTreeOptions>, DeleteExperiment, false,
               false, true, false>;
BENCHMARK_DEFINE_F(DeleteZDSTFixture, BM_DST_Deletion)(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto i : this->experiment_indices) {
			this->t.remove(this->fixed_nodes[i]);
		}
		this->papi.stop();

		state.PauseTiming();
		for (auto i : this->experiment_indices) {
			this->t.insert(this->fixed_nodes[i]);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(DeleteZDSTFixture, BM_DST_Deletion)

/*
 * WB-DST with 3/2
 */
using Delete32WBDSTFixture = DSTFixture<
    WBDSTInterface<BasicDSTTreeOptions, ygg::TreeFlags::WBT_DELTA_NUMERATOR<3>,
                   ygg::TreeFlags::WBT_DELTA_DENOMINATOR<1>,
                   ygg::TreeFlags::WBT_GAMMA_NUMERATOR<2>,
                   ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<1>>,
    DeleteExperiment, false, false, true, false>;
BENCHMARK_DEFINE_F(Delete32WBDSTFixture, BM_DST_Deletion)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto i : this->experiment_indices) {
			this->t.remove(this->fixed_nodes[i]);
		}
		this->papi.stop();

		state.PauseTiming();
		for (auto i : this->experiment_indices) {
			this->t.insert(this->fixed_nodes[i]);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(Delete32WBDSTFixture, BM_DST_Deletion)

/*
 * WB-DST with 2,3/2
 */
using DeleteBalWBDSTFixture = DSTFixture<
    WBDSTInterface<BasicDSTTreeOptions, ygg::TreeFlags::WBT_DELTA_NUMERATOR<2>,
                   ygg::TreeFlags::WBT_DELTA_DENOMINATOR<1>,
                   ygg::TreeFlags::WBT_GAMMA_NUMERATOR<3>,
                   ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<2>>,
    DeleteExperiment, false, false, true, false>;
BENCHMARK_DEFINE_F(DeleteBalWBDSTFixture, BM_DST_Deletion)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (auto i : this->experiment_indices) {
			this->t.remove(this->fixed_nodes[i]);
		}
		this->papi.stop();

		state.PauseTiming();
		for (auto i : this->experiment_indices) {
			this->t.insert(this->fixed_nodes[i]);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(DeleteBalWBDSTFixture, BM_DST_Deletion)

#ifndef NOMAIN
#include "main.hpp"
#endif

#endif
