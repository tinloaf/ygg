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
	PointerCountCallback::reset();
	Clock c;
	for (auto _ : state) {
		PointerCountCallback::start();
		c.start();
		this->papi.start();
		for (auto & n : this->experiment_nodes) {
			this->t.insert(n);
		}
		this->papi.stop();
		PointerCountCallback::stop();
		state.SetIterationTime(c.get());

		for (auto & n : this->experiment_nodes) {
			this->t.remove(n);
		}
	}

	this->papi.report_and_reset(state);
	PointerCountCallback::report(state);
}
REGISTER(InsertRBDSTFixture, BM_DST_Insertion)

/*
 * Zip DST with Hashing
 */
using InsertZHDSTInterface =
    ZDSTInterface<BasicDSTTreeOptions,
                  ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT<
                      16186402584962403883ul>,
                  ygg::TreeFlags::ZTREE_USE_HASH>;
using InsertZHDSTFixture = DSTFixture<InsertZHDSTInterface, InsertExperiment,
                                      true, false, false, false>;
BENCHMARK_DEFINE_F(InsertZHDSTFixture, BM_DST_Insertion)
(benchmark::State & state)
{
	PointerCountCallback::reset();
	Clock c;
	for (auto _ : state) {
		PointerCountCallback::start();
		c.start();
		this->papi.start();
		for (auto & n : this->experiment_nodes) {
			this->t.insert(n);
		}
		this->papi.stop();
		PointerCountCallback::stop();
		state.SetIterationTime(c.get());

		InsertZHDSTInterface::report_ranks(state, this->t);
		for (auto & n : this->experiment_nodes) {
			this->t.remove(n);
		}
	}

	this->papi.report_and_reset(state);
	PointerCountCallback::report(state);
}
REGISTER(InsertZHDSTFixture, BM_DST_Insertion)

/*
 * Zip DST with Hashing and storing the ranks
 */
using InsertZHSDSTInterface =
    ZDSTInterface<BasicDSTTreeOptions,
                  ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT<
                      16186402584962403883ul>,
                  ygg::TreeFlags::ZTREE_RANK_TYPE<std::uint8_t>,
                  ygg::TreeFlags::ZTREE_USE_HASH>;
using InsertZHSDSTFixture = DSTFixture<InsertZHSDSTInterface, InsertExperiment,
                                       true, false, false, false>;
BENCHMARK_DEFINE_F(InsertZHSDSTFixture, BM_DST_Insertion)
(benchmark::State & state)
{
	PointerCountCallback::reset();
	Clock c;
	for (auto _ : state) {
		PointerCountCallback::start();
		c.start();
		this->papi.start();
		for (auto & n : this->experiment_nodes) {
			this->t.insert(n);
		}
		this->papi.stop();
		PointerCountCallback::stop();
		state.SetIterationTime(c.get());

		InsertZHSDSTInterface::report_ranks(state, this->t);
		for (auto & n : this->experiment_nodes) {
			this->t.remove(n);
		}
	}

	this->papi.report_and_reset(state);
	PointerCountCallback::report(state);
}
REGISTER(InsertZHSDSTFixture, BM_DST_Insertion)

/*
 * Zip DST with truly random ranks
 */
using InsertZRDSTInterface =
    ZDSTInterface<BasicDSTTreeOptions,
                  ygg::TreeFlags::ZTREE_RANK_TYPE<uint8_t>>;
using InsertZRDSTFixture = DSTFixture<InsertZRDSTInterface, InsertExperiment,
                                      true, false, false, false>;
BENCHMARK_DEFINE_F(InsertZRDSTFixture, BM_DST_Insertion)
(benchmark::State & state)
{
	PointerCountCallback::reset();
	Clock c;
	for (auto _ : state) {
		PointerCountCallback::start();
		c.start();
		this->papi.start();
		for (auto & n : this->experiment_nodes) {

			// To also measure the overhead of randomly generating the ranks, we have
			// to update the ranks here. For hashing-based Zip Trees, the DST does
			// this automatically in insert().
			n.benchmark_update_inner_ranks();

			this->t.insert(n);
		}
		this->papi.stop();
		PointerCountCallback::stop();
		state.SetIterationTime(c.get());

		InsertZRDSTInterface::report_ranks(state, this->t);
		for (auto & n : this->experiment_nodes) {
			this->t.remove(n);
		}
	}

	this->papi.report_and_reset(state);
	PointerCountCallback::report(state);
}
REGISTER(InsertZRDSTFixture, BM_DST_Insertion)

/*
 * WBT-DST with 3/2
 */
using Insert32WBDSTFixture = DSTFixture<
    WBDSTInterface<BasicDSTTreeOptions, ygg::TreeFlags::WBT_DELTA_NUMERATOR<3>,
                   ygg::TreeFlags::WBT_DELTA_DENOMINATOR<1>,
                   ygg::TreeFlags::WBT_GAMMA_NUMERATOR<2>,
                   ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<1>,
                   ygg::TreeFlags::WBT_SINGLE_PASS>,
    InsertExperiment, true, false, false, false>;
BENCHMARK_DEFINE_F(Insert32WBDSTFixture, BM_DST_Insertion)
(benchmark::State & state)
{
	PointerCountCallback::reset();
	Clock c;
	for (auto _ : state) {
		PointerCountCallback::start();
		c.start();
		this->papi.start();
		for (auto & n : this->experiment_nodes) {
			this->t.insert(n);
		}
		this->papi.stop();
		PointerCountCallback::stop();
		state.SetIterationTime(c.get());

		for (auto & n : this->experiment_nodes) {
			this->t.remove(n);
		}
	}

	this->papi.report_and_reset(state);
	PointerCountCallback::report(state);
}
REGISTER(Insert32WBDSTFixture, BM_DST_Insertion)

/*
 * WBT-DST with 2,3/2
 */
using InsertBalWBDSTFixture = DSTFixture<
    WBDSTInterface<BasicDSTTreeOptions, ygg::TreeFlags::WBT_DELTA_NUMERATOR<2>,
                   ygg::TreeFlags::WBT_DELTA_DENOMINATOR<1>,
                   ygg::TreeFlags::WBT_GAMMA_NUMERATOR<3>,
                   ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<2>,
                   ygg::TreeFlags::WBT_SINGLE_PASS>,
    InsertExperiment, true, false, false, false>;
BENCHMARK_DEFINE_F(InsertBalWBDSTFixture, BM_DST_Insertion)
(benchmark::State & state)
{
	PointerCountCallback::reset();
	Clock c;
	for (auto _ : state) {
		PointerCountCallback::start();
		c.start();
		this->papi.start();
		for (auto & n : this->experiment_nodes) {
			this->t.insert(n);
		}
		this->papi.stop();
		PointerCountCallback::stop();
		state.SetIterationTime(c.get());

		for (auto & n : this->experiment_nodes) {
			this->t.remove(n);
		}
	}

	this->papi.report_and_reset(state);
	PointerCountCallback::report(state);
}
REGISTER(InsertBalWBDSTFixture, BM_DST_Insertion)

#ifndef NOMAIN
#include "main.hpp"
#endif

#endif
