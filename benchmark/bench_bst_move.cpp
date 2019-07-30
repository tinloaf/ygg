#ifndef BENCH_BST_MOVE_HPP
#define BENCH_BST_MOVE_HPP

#include "common_bst.hpp"

/*
 * Ygg's Red-Black Tree
 */
using MoveYggRBBSTFixture =
    BSTFixture<YggRBTreeInterface<BasicTreeOptions>, MoveExperiment, UseUniform,
               UseNone, UseUniform, DYN_GENERATOR, false, false, 10>;
BENCHMARK_DEFINE_F(MoveYggRBBSTFixture, BM_BST_Move)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (size_t i = 0; i < this->experiment_node_pointers.size(); i++) {
			auto * n = this->experiment_node_pointers[i];
			auto new_val = this->experiment_values[i];

			this->t.remove(*n);
			NodeInterface::set_value(*n, new_val);
			this->t.insert(*n);
		}
		this->papi.stop();
		state.PauseTiming();

		for (size_t i = 0; i < this->experiment_node_pointers.size(); i++) {
			auto * n = this->experiment_node_pointers[i];
			auto old_val = this->fixed_values[i];

			this->t.remove(*n);
			NodeInterface::set_value(*n, old_val);
			this->t.insert(*n);
		}

		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(MoveYggRBBSTFixture, BM_BST_Move)

/*
 * Ygg's Red-Black Tree, avoiding conditional branches
 */
using MoveYggRBBSTFixtureArith =
    BSTFixture<YggRBTreeInterface<ArithTreeOptions, RBBSTNamerArith>,
               MoveExperiment, UseUniform, UseNone, UseUniform, DYN_GENERATOR,
               false, false, 10>;
BENCHMARK_DEFINE_F(MoveYggRBBSTFixtureArith, BM_BST_Move)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (size_t i = 0; i < this->experiment_node_pointers.size(); i++) {
			auto * n = this->experiment_node_pointers[i];
			auto new_val = this->experiment_values[i];

			this->t.remove(*n);
			NodeInterface::set_value(*n, new_val);
			this->t.insert(*n);
		}
		this->papi.stop();
		state.PauseTiming();

		for (size_t i = 0; i < this->experiment_node_pointers.size(); i++) {
			auto * n = this->experiment_node_pointers[i];
			auto old_val = this->fixed_values[i];

			this->t.remove(*n);
			NodeInterface::set_value(*n, old_val);
			this->t.insert(*n);
		}

		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(MoveYggRBBSTFixtureArith, BM_BST_Move)

/*
 * Ygg's Weight-Balanced Tree
 */
// Default gamma, delta / twopass
using MoveYggWBDefGDefDTPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTTwopassTreeOptions, WBBSTNamerDefGDefDTP>,
               MoveExperiment, UseUniform, UseNone, UseUniform, DYN_GENERATOR,
               false, false, 10>;
BENCHMARK_DEFINE_F(MoveYggWBDefGDefDTPBSTFixture, BM_BST_Move)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (size_t i = 0; i < this->experiment_node_pointers.size(); i++) {
			auto * n = this->experiment_node_pointers[i];
			auto new_val = this->experiment_values[i];

			this->t.remove(*n);
			NodeInterface::set_value(*n, new_val);
			this->t.insert(*n);
		}
		this->papi.stop();
		state.PauseTiming();

		for (size_t i = 0; i < this->experiment_node_pointers.size(); i++) {
			auto * n = this->experiment_node_pointers[i];
			auto old_val = this->fixed_values[i];

			this->t.remove(*n);
			NodeInterface::set_value(*n, old_val);
			this->t.insert(*n);
		}

		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(MoveYggWBDefGDefDTPBSTFixture, BM_BST_Move)

// Default gamma, delta / single pass
using MoveYggWBDefGDefDSPBSTFixture = BSTFixture<
    YggWBTreeInterface<WBTSinglepassTreeOptions, WBBSTNamerDefGDefDSP>,
    MoveExperiment, UseUniform, UseUniform, UseNone, UseNone, false>;
BENCHMARK_DEFINE_F(MoveYggWBDefGDefDSPBSTFixture, BM_BST_Move)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (size_t i = 0; i < this->experiment_node_pointers.size(); i++) {
			auto * n = this->experiment_node_pointers[i];
			auto new_val = this->experiment_values[i];

			this->t.remove(*n);
			NodeInterface::set_value(*n, new_val);
			this->t.insert(*n);
		}
		this->papi.stop();
		state.PauseTiming();

		for (size_t i = 0; i < this->experiment_node_pointers.size(); i++) {
			auto * n = this->experiment_node_pointers[i];
			auto old_val = this->fixed_values[i];

			this->t.remove(*n);
			NodeInterface::set_value(*n, old_val);
			this->t.insert(*n);
		}

		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(MoveYggWBDefGDefDSPBSTFixture, BM_BST_Move)

// Lai and Wood gamma, delta / single pass
using MoveYggWBLWSPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTSinglepassLWTreeOptions, WBBSTNamerLWSP>,
               MoveExperiment, UseUniform, UseNone, UseUniform, DYN_GENERATOR,
               false, false, 10>;
BENCHMARK_DEFINE_F(MoveYggWBLWSPBSTFixture, BM_BST_Move)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (size_t i = 0; i < this->experiment_node_pointers.size(); i++) {
			auto * n = this->experiment_node_pointers[i];
			auto new_val = this->experiment_values[i];

			this->t.remove(*n);
			NodeInterface::set_value(*n, new_val);
			this->t.insert(*n);
		}
		this->papi.stop();
		state.PauseTiming();

		for (size_t i = 0; i < this->experiment_node_pointers.size(); i++) {
			auto * n = this->experiment_node_pointers[i];
			auto old_val = this->fixed_values[i];

			this->t.remove(*n);
			NodeInterface::set_value(*n, old_val);
			this->t.insert(*n);
		}

		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(MoveYggWBLWSPBSTFixture, BM_BST_Move)

// Balance-focussed gamma, delta / single pass
using MoveYggWBBalSPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTSinglepassBalTreeOptions, WBBSTNamerBalSP>,
               MoveExperiment, UseUniform, UseNone, UseUniform, DYN_GENERATOR,
               false, false, 10>;
BENCHMARK_DEFINE_F(MoveYggWBBalSPBSTFixture, BM_BST_Move)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (size_t i = 0; i < this->experiment_node_pointers.size(); i++) {
			auto * n = this->experiment_node_pointers[i];
			auto new_val = this->experiment_values[i];

			this->t.remove(*n);
			NodeInterface::set_value(*n, new_val);
			this->t.insert(*n);
		}
		this->papi.stop();
		state.PauseTiming();

		for (size_t i = 0; i < this->experiment_node_pointers.size(); i++) {
			auto * n = this->experiment_node_pointers[i];
			auto old_val = this->fixed_values[i];

			this->t.remove(*n);
			NodeInterface::set_value(*n, old_val);
			this->t.insert(*n);
		}

		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(MoveYggWBBalSPBSTFixture, BM_BST_Move)

// Balance-focussed gamma, delta / single pass, avoiding conditionals
using MoveYggWBBalSPArithBSTFixture = BSTFixture<
    YggWBTreeInterface<WBTSinglepassBalArithTreeOptions, WBBSTNamerBalSPArith>,
    MoveExperiment, UseUniform, UseUniform, UseNone, UseNone, false>;
BENCHMARK_DEFINE_F(MoveYggWBBalSPArithBSTFixture, BM_BST_Move)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (size_t i = 0; i < this->experiment_node_pointers.size(); i++) {
			auto * n = this->experiment_node_pointers[i];
			auto new_val = this->experiment_values[i];

			this->t.remove(*n);
			NodeInterface::set_value(*n, new_val);
			this->t.insert(*n);
		}
		this->papi.stop();
		state.PauseTiming();

		for (size_t i = 0; i < this->experiment_node_pointers.size(); i++) {
			auto * n = this->experiment_node_pointers[i];
			auto old_val = this->fixed_values[i];

			this->t.remove(*n);
			NodeInterface::set_value(*n, old_val);
			this->t.insert(*n);
		}

		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(MoveYggWBBalSPArithBSTFixture, BM_BST_Move)

// integral gamma, delta / single pass
using MoveYggWB3G2DSPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTSinglepass32TreeOptions, WBBSTNamer3G2DSP>,
               MoveExperiment, UseUniform, UseNone, UseUniform, DYN_GENERATOR,
               false, false, 10>;
BENCHMARK_DEFINE_F(MoveYggWB3G2DSPBSTFixture, BM_BST_Move)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (size_t i = 0; i < this->experiment_node_pointers.size(); i++) {
			auto * n = this->experiment_node_pointers[i];
			auto new_val = this->experiment_values[i];

			this->t.remove(*n);
			NodeInterface::set_value(*n, new_val);
			this->t.insert(*n);
		}
		this->papi.stop();
		state.PauseTiming();

		for (size_t i = 0; i < this->experiment_node_pointers.size(); i++) {
			auto * n = this->experiment_node_pointers[i];
			auto old_val = this->fixed_values[i];

			this->t.remove(*n);
			NodeInterface::set_value(*n, old_val);
			this->t.insert(*n);
		}

		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(MoveYggWB3G2DSPBSTFixture, BM_BST_Move)

// integral gamma, delta / twopass
using MoveYggWB3G2DTPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTTwopass32TreeOptions, WBBSTNamer3G2DTP>,
               MoveExperiment, UseUniform, UseNone, UseUniform, DYN_GENERATOR,
               false, false, 10>;
BENCHMARK_DEFINE_F(MoveYggWB3G2DTPBSTFixture, BM_BST_Move)
(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi.start();
		for (size_t i = 0; i < this->experiment_node_pointers.size(); i++) {
			auto * n = this->experiment_node_pointers[i];
			auto new_val = this->experiment_values[i];

			this->t.remove(*n);
			NodeInterface::set_value(*n, new_val);
			this->t.insert(*n);
		}
		this->papi.stop();
		state.PauseTiming();

		for (size_t i = 0; i < this->experiment_node_pointers.size(); i++) {
			auto * n = this->experiment_node_pointers[i];
			auto old_val = this->fixed_values[i];

			this->t.remove(*n);
			NodeInterface::set_value(*n, old_val);
			this->t.insert(*n);
		}

		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(MoveYggWB3G2DTPBSTFixture, BM_BST_Move)

#ifndef NOMAIN
#include "main.hpp"
#endif

#endif
