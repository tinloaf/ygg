#ifndef BENCH_DST_DELETE_HPP
#define BENCH_DST_DELETE_HPP

#include "common_dst.hpp"

/*
 * Red-Black DST
 */
using DeleteRBDSTFixture =
	DSTFixture<RBDSTInterface<BasicDSTTreeOptions>, DeleteExperiment, false, false, true, false>;
BENCHMARK_DEFINE_F(DeleteRBDSTFixture, BM_DST_Deletion)(benchmark::State & state)
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
	DSTFixture<ZDSTInterface<BasicDSTTreeOptions>, DeleteExperiment, false, false, true, false>;
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

#ifndef NOMAIN
#include "main.hpp"
#endif

#endif
