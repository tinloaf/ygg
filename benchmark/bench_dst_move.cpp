#ifndef BENCH_DST_INSERT_HPP
#define BENCH_DST_INSERT_HPP

#include "common_dst.hpp"

/*
 * Red-Black DST
 */
using RBDSTFixture =
	Fixture<RBDSTInterface<BasicTreeOptions>, false, true, true, false>;
BENCHMARK_DEFINE_F(RBDSTFixture, BM_Move)(benchmark::State & state)
{
	for (auto _ : state) {
		for (auto i : this->experiment_indices) {
			this->t.remove(this->fixed_nodes[i]);
			auto [upper, lower, value] = this->experiment_values[i];
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
BENCHMARK_REGISTER_F(RBDSTFixture, BM_Move)->Args({1000, 1000});

/*
 * Zip DST
 */
using ZDSTFixture =
	Fixture<ZDSTInterface<BasicTreeOptions>, false, true, true, false>;
BENCHMARK_DEFINE_F(ZDSTFixture, BM_Move)(benchmark::State & state)
{
	for (auto _ : state) {
		for (auto i : this->experiment_indices) {
			this->t.remove(this->fixed_nodes[i]);
			auto [upper, lower, value] = this->experiment_values[i];
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
BENCHMARK_REGISTER_F(ZDSTFixture, BM_Move)->Args({1000, 1000});


BENCHMARK_MAIN();

#endif
