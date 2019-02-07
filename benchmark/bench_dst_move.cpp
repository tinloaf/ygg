#ifndef BENCH_DST_MOVE_HPP
#define BENCH_DST_MOVE_HPP

#include "common_dst.hpp"

/*
 * Red-Black DST
 */
using MoveRBDSTFixture =
    DSTFixture<RBDSTInterface<BasicDSTTreeOptions>, false, true, true, false>;
BENCHMARK_DEFINE_F(MoveRBDSTFixture, BM_DST_Move)(benchmark::State & state)
{
	for (auto _ : state) {
		size_t j = 0;
		for (auto i : this->experiment_indices) {
			this->t.remove(this->fixed_nodes[i]);
			auto [upper, lower, value] = this->experiment_values[j++];
			this->fixed_nodes[i].upper = upper;
			this->fixed_nodes[i].lower = lower;
			this->fixed_nodes[i].value = value; // TODO don't do this?
			this->t.insert(this->fixed_nodes[i]);
		}

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
}
BENCHMARK_REGISTER_F(MoveRBDSTFixture, BM_DST_Move)
    ->RangeMultiplier(2)
    ->Ranges({{BASE_SIZE, BASE_SIZE * (1 << DOUBLINGS)}, {EXPERIMENT_SIZE, EXPERIMENT_SIZE}});

/*
 * Zip DST
 */
using MoveZDSTFixture =
    DSTFixture<ZDSTInterface<BasicDSTTreeOptions>, false, true, true, false>;
BENCHMARK_DEFINE_F(MoveZDSTFixture, BM_DST_Move)(benchmark::State & state)
{
	for (auto _ : state) {
		size_t j = 0;
		for (auto i : this->experiment_indices) {
			this->t.remove(this->fixed_nodes[i]);
			auto [upper, lower, value] = this->experiment_values[j++];
			this->fixed_nodes[i].upper = upper;
			this->fixed_nodes[i].lower = lower;
			this->fixed_nodes[i].value = value; // TODO don't do this?
			this->t.insert(this->fixed_nodes[i]);
		}

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
}
BENCHMARK_REGISTER_F(MoveZDSTFixture, BM_DST_Move)
    ->RangeMultiplier(2)
    ->Ranges({{BASE_SIZE, BASE_SIZE * (1 << DOUBLINGS)}, {EXPERIMENT_SIZE, EXPERIMENT_SIZE}});

#endif
