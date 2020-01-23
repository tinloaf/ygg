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
	PointerCountCallback::reset();
	Clock c;
	for (auto _ : state) {
		size_t j = 0;
		PointerCountCallback::start();
		c.start();
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
		state.SetIterationTime(c.get());
		PointerCountCallback::stop();

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
	}

	this->papi.report_and_reset(state);
	PointerCountCallback::report(state);
}
REGISTER(MoveRBDSTFixture, BM_DST_Move)

/*
 * Zip DST with hashing
 */
using MoveZHDSTInterface = ZDSTInterface<
    BasicDSTTreeOptions,
    ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT<3445358421>,
    ygg::TreeFlags::ZTREE_USE_HASH>;
using MoveZHDSTFixture =
    DSTFixture<MoveZHDSTInterface, MoveExperiment, false, true, true, false>;
BENCHMARK_DEFINE_F(MoveZHDSTFixture, BM_DST_Move)(benchmark::State & state)
{
	PointerCountCallback::reset();
	Clock c;
	for (auto _ : state) {
		size_t j = 0;
		PointerCountCallback::start();
		c.start();
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
		state.SetIterationTime(c.get());
		PointerCountCallback::stop();

		MoveZHDSTInterface::report_ranks(state, this->t);

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
	}

	this->papi.report_and_reset(state);
	PointerCountCallback::report(state);
}
REGISTER(MoveZHDSTFixture, BM_DST_Move)

/*
 * Zip DST with truly random ranks
 */
using MoveZRDSTInterface =
    ZDSTInterface<BasicDSTTreeOptions,
                  ygg::TreeFlags::ZTREE_RANK_TYPE<uint8_t>>;
using MoveZRDSTFixture =
    DSTFixture<MoveZRDSTInterface, MoveExperiment, false, true, true, false>;
BENCHMARK_DEFINE_F(MoveZRDSTFixture, BM_DST_Move)(benchmark::State & state)
{
	PointerCountCallback::reset();
	Clock c;
	for (auto _ : state) {
		size_t j = 0;
		PointerCountCallback::start();
		c.start();
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
		state.SetIterationTime(c.get());
		PointerCountCallback::stop();

		MoveZRDSTInterface::report_ranks(state, this->t);

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
	}

	this->papi.report_and_reset(state);
	PointerCountCallback::report(state);
}
REGISTER(MoveZRDSTFixture, BM_DST_Move)

/*
 * WB-DST with 3/2
 */
using Move32WBDSTFixture = DSTFixture<
    WBDSTInterface<BasicDSTTreeOptions, ygg::TreeFlags::WBT_DELTA_NUMERATOR<3>,
                   ygg::TreeFlags::WBT_DELTA_DENOMINATOR<1>,
                   ygg::TreeFlags::WBT_GAMMA_NUMERATOR<2>,
                   ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<1>,
                   ygg::TreeFlags::WBT_SINGLE_PASS>,
    MoveExperiment, false, true, true, false>;
BENCHMARK_DEFINE_F(Move32WBDSTFixture, BM_DST_Move)(benchmark::State & state)
{
	PointerCountCallback::reset();
	Clock c;
	for (auto _ : state) {
		size_t j = 0;
		PointerCountCallback::start();
		c.start();
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
		state.SetIterationTime(c.get());
		PointerCountCallback::stop();

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
	}

	this->papi.report_and_reset(state);
	PointerCountCallback::report(state);
}
REGISTER(Move32WBDSTFixture, BM_DST_Move)

/*
 * WB-DST with 2,3/2
 */
using MoveBalWBDSTFixture = DSTFixture<
    WBDSTInterface<BasicDSTTreeOptions, ygg::TreeFlags::WBT_DELTA_NUMERATOR<2>,
                   ygg::TreeFlags::WBT_DELTA_DENOMINATOR<1>,
                   ygg::TreeFlags::WBT_GAMMA_NUMERATOR<3>,
                   ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<2>,
                   ygg::TreeFlags::WBT_SINGLE_PASS>,
    MoveExperiment, false, true, true, false>;
BENCHMARK_DEFINE_F(MoveBalWBDSTFixture, BM_DST_Move)(benchmark::State & state)
{
	PointerCountCallback::reset();
	Clock c;
	for (auto _ : state) {
		size_t j = 0;
		PointerCountCallback::start();
		c.start();
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
		state.SetIterationTime(c.get());
		PointerCountCallback::stop();

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
	}

	this->papi.report_and_reset(state);
	PointerCountCallback::report(state);
}
REGISTER(MoveBalWBDSTFixture, BM_DST_Move)

#ifndef NOMAIN
#include "main.hpp"
#endif

#endif
