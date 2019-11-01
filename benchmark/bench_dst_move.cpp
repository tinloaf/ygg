#ifndef BENCH_DST_MOVE_HPP
#define BENCH_DST_MOVE_HPP

#include "common_dst.hpp"

/*
 * Red-Black DST
 */
using MoveRBDSTFixture = DSTFixture<RBDSTInterface<BasicDSTTreeOptions>,
                                    MoveExperiment, false, true, true, false>;
BENCHMARK_DEFINE_F(MoveRBDSTFixture, BM_DST_Move)(benchmark::State & state)
{
	for (auto _ : state) {
		size_t j = 0;
		this->papi.start();
		for (auto i : this->experiment_indices) {
			this->t.remove(this->fixed_nodes[i]);
			auto [upper, lower, value] = this->experiment_values[j++];
			this->fixed_nodes[i].upper = upper;
			this->fixed_nodes[i].lower = lower;
			this->fixed_nodes[i].value = value; // TODO don't do this?
			this->t.insert(this->fixed_nodes[i]);
		}
		this->papi.stop();

		state.PauseTiming();
		// TODO actually, this is the same as above - also time it?
		for (auto i : this->experiment_indices) {
			this->t.remove(this->fixed_nodes[i]);
			auto [upper, lower, value] = this->fixed_values[i];
			this->fixed_nodes[i].upper = upper;
			this->fixed_nodes[i].lower = lower;
			this->fixed_nodes[i].value = value; // TODO don't do this?
			this->t.insert(this->fixed_nodes[i]);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(MoveRBDSTFixture, BM_DST_Move)

/*
 * Weight-Balanced DST
 */
using MoveWBDSTFixture = DSTFixture<WBDSTInterface<BasicDSTTreeOptions>,
                                    MoveExperiment, false, true, true, false>;
BENCHMARK_DEFINE_F(MoveWBDSTFixture, BM_DST_Move)(benchmark::State & state)
{
	for (auto _ : state) {
		size_t j = 0;
		this->papi.start();
		for (auto i : this->experiment_indices) {
			this->t.remove(this->fixed_nodes[i]);
			auto [upper, lower, value] = this->experiment_values[j++];
			this->fixed_nodes[i].upper = upper;
			this->fixed_nodes[i].lower = lower;
			this->fixed_nodes[i].value = value; // TODO don't do this?
			this->t.insert(this->fixed_nodes[i]);
		}
		this->papi.stop();

		state.PauseTiming();
		// TODO actually, this is the same as above - also time it?
		for (auto i : this->experiment_indices) {
			this->t.remove(this->fixed_nodes[i]);
			auto [upper, lower, value] = this->fixed_values[i];
			this->fixed_nodes[i].upper = upper;
			this->fixed_nodes[i].lower = lower;
			this->fixed_nodes[i].value = value; // TODO don't do this?
			this->t.insert(this->fixed_nodes[i]);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(MoveWBDSTFixture, BM_DST_Move)

/*
 * Zip DST
 */
using MoveZDSTFixture = DSTFixture<ZDSTInterface<BasicDSTTreeOptions>,
                                   MoveExperiment, false, true, true, false>;
BENCHMARK_DEFINE_F(MoveZDSTFixture, BM_DST_Move)(benchmark::State & state)
{
	for (auto _ : state) {
		size_t j = 0;
		this->papi.start();
		for (auto i : this->experiment_indices) {
			this->t.remove(this->fixed_nodes[i]);
			auto [upper, lower, value] = this->experiment_values[j++];
			this->fixed_nodes[i].upper = upper;
			this->fixed_nodes[i].lower = lower;
			this->fixed_nodes[i].value = value; // TODO don't do this?
			this->t.insert(this->fixed_nodes[i]);
		}
		this->papi.stop();

		state.PauseTiming();
		// TODO actually, this is the same as above - also time it?
		for (auto i : this->experiment_indices) {
			this->t.remove(this->fixed_nodes[i]);
			auto [upper, lower, value] = this->fixed_values[i];
			this->fixed_nodes[i].upper = upper;
			this->fixed_nodes[i].lower = lower;
			this->fixed_nodes[i].value = value; // TODO don't do this?
			this->t.insert(this->fixed_nodes[i]);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(MoveZDSTFixture, BM_DST_Move)

/*
 * WB-DST with 3/2
 */
using Move32WBDSTFixture = DSTFixture<
    WBDSTInterface<BasicDSTTreeOptions, ygg::TreeFlags::WBT_DELTA_NUMERATOR<3>,
                   ygg::TreeFlags::WBT_DELTA_DENOMINATOR<1>,
                   ygg::TreeFlags::WBT_GAMMA_NUMERATOR<2>,
                   ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<1>>,
    MoveExperiment, false, true, true, false>;
BENCHMARK_DEFINE_F(Move32WBDSTFixture, BM_DST_Move)(benchmark::State & state)
{
	for (auto _ : state) {
		size_t j = 0;
		this->papi.start();
		for (auto i : this->experiment_indices) {
			this->t.remove(this->fixed_nodes[i]);
			auto [upper, lower, value] = this->experiment_values[j++];
			this->fixed_nodes[i].upper = upper;
			this->fixed_nodes[i].lower = lower;
			this->fixed_nodes[i].value = value; // TODO don't do this?
			this->t.insert(this->fixed_nodes[i]);
		}
		this->papi.stop();

		state.PauseTiming();
		// TODO actually, this is the same as above - also time it?
		for (auto i : this->experiment_indices) {
			this->t.remove(this->fixed_nodes[i]);
			auto [upper, lower, value] = this->fixed_values[i];
			this->fixed_nodes[i].upper = upper;
			this->fixed_nodes[i].lower = lower;
			this->fixed_nodes[i].value = value; // TODO don't do this?
			this->t.insert(this->fixed_nodes[i]);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(Move32WBDSTFixture, BM_DST_Move)

#ifndef NOMAIN
#include "main.hpp"
#endif

#endif
