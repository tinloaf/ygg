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
	PointerCountCallback::reset();
	for (auto _ : state) {
		PointerCountCallback::start();
		this->papi.start();
		for (auto i : this->experiment_indices) {
			this->t.remove(this->fixed_nodes[i]);
		}
		this->papi.stop();
		PointerCountCallback::stop();

		state.PauseTiming();
		for (auto i : this->experiment_indices) {
			this->t.insert(this->fixed_nodes[i]);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
	PointerCountCallback::report(state);
}

/*
 * Zip DST with hashing
 */
using DeleteZHDSTFixture = DSTFixture<
    ZDSTInterface<
        BasicDSTTreeOptions,
        ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT<3445358421>,
        ygg::TreeFlags::ZTREE_USE_HASH>,
    DeleteExperiment, false, false, true, false>;
BENCHMARK_DEFINE_F(DeleteZHDSTFixture, BM_DST_Deletion)
(benchmark::State & state)
{
	PointerCountCallback::reset();
	for (auto _ : state) {
		PointerCountCallback::start();
		this->papi.start();
		for (auto i : this->experiment_indices) {
			this->t.remove(this->fixed_nodes[i]);
		}
		this->papi.stop();
		PointerCountCallback::stop();

		state.PauseTiming();
		for (auto i : this->experiment_indices) {
			this->t.insert(this->fixed_nodes[i]);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
	PointerCountCallback::report(state);
}

/*
 * Zip DST with truly random ranks
 */
using DeleteZRDSTFixture =
    DSTFixture<ZDSTInterface<BasicDSTTreeOptions,
                             ygg::TreeFlags::ZTREE_RANK_TYPE<uint8_t>>,
               DeleteExperiment, false, false, true, false>;
BENCHMARK_DEFINE_F(DeleteZRDSTFixture, BM_DST_Deletion)
(benchmark::State & state)
{
	PointerCountCallback::reset();
	for (auto _ : state) {
		PointerCountCallback::start();
		this->papi.start();
		for (auto i : this->experiment_indices) {
			this->t.remove(this->fixed_nodes[i]);
		}
		this->papi.stop();
		PointerCountCallback::stop();

		state.PauseTiming();
		for (auto i : this->experiment_indices) {
			this->t.insert(this->fixed_nodes[i]);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
	PointerCountCallback::report(state);
}

/*
 * WB-DST with 3/2
 */
using Delete32WBDSTFixture = DSTFixture<
    WBDSTInterface<BasicDSTTreeOptions, ygg::TreeFlags::WBT_DELTA_NUMERATOR<3>,
                   ygg::TreeFlags::WBT_DELTA_DENOMINATOR<1>,
                   ygg::TreeFlags::WBT_GAMMA_NUMERATOR<2>,
                   ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<1>,
                   ygg::TreeFlags::WBT_SINGLE_PASS>,
    DeleteExperiment, false, false, true, false>;
BENCHMARK_DEFINE_F(Delete32WBDSTFixture, BM_DST_Deletion)
(benchmark::State & state)
{
	PointerCountCallback::reset();
	for (auto _ : state) {
		PointerCountCallback::start();
		this->papi.start();
		for (auto i : this->experiment_indices) {
			this->t.remove(this->fixed_nodes[i]);
		}
		this->papi.stop();
		PointerCountCallback::stop();

		state.PauseTiming();
		for (auto i : this->experiment_indices) {
			this->t.insert(this->fixed_nodes[i]);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
	PointerCountCallback::report(state);
}

/*
 * WB-DST with 2,3/2
 */
using DeleteBalWBDSTFixture = DSTFixture<
    WBDSTInterface<BasicDSTTreeOptions, ygg::TreeFlags::WBT_DELTA_NUMERATOR<2>,
                   ygg::TreeFlags::WBT_DELTA_DENOMINATOR<1>,
                   ygg::TreeFlags::WBT_GAMMA_NUMERATOR<3>,
                   ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<2>,
                   ygg::TreeFlags::WBT_SINGLE_PASS>,
    DeleteExperiment, false, false, true, false>;
BENCHMARK_DEFINE_F(DeleteBalWBDSTFixture, BM_DST_Deletion)
(benchmark::State & state)
{
	PointerCountCallback::reset();
	for (auto _ : state) {
		PointerCountCallback::start();
		this->papi.start();
		for (auto i : this->experiment_indices) {
			this->t.remove(this->fixed_nodes[i]);
		}
		this->papi.stop();
		PointerCountCallback::stop();

		state.PauseTiming();
		for (auto i : this->experiment_indices) {
			this->t.insert(this->fixed_nodes[i]);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
	PointerCountCallback::report(state);
}

REGISTER(DeleteRBDSTFixture, BM_DST_Deletion)
REGISTER(DeleteZRDSTFixture, BM_DST_Deletion)
REGISTER(DeleteZHDSTFixture, BM_DST_Deletion)
REGISTER(Delete32WBDSTFixture, BM_DST_Deletion)
REGISTER(DeleteBalWBDSTFixture, BM_DST_Deletion)

#ifndef NOMAIN
#include "main.hpp"
#endif

#endif
