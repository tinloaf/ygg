#ifndef BENCH_BST_INSERT_HPP
#define BENCH_BST_INSERT_HPP

#include "common_bst.hpp"

struct BSTInsertOptions : public DefaultBenchmarkOptions
{
	using MainRandomizer = UseUniform;
	constexpr static bool need_nodes = true;
	using NodeRandomizer = UseUniform;
#ifdef PRESORT
	constexpr static bool nodes_presort = true;
	constexpr static double nodes_presort_fraction = 0.5;
	constexpr static bool fixed_presort = true;
	constexpr static double fixed_presort_fraction = 0.5;
#endif
};

/*
 * Ygg's Red-Black Tree
 */
using InsertYggRBBSTFixture = BSTFixture<YggRBTreeInterface<BasicTreeOptions>,
                                         InsertExperiment, BSTInsertOptions>;
BENCHMARK_DEFINE_F(InsertYggRBBSTFixture, BM_BST_Insertion)
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
REGISTER(InsertYggRBBSTFixture, BM_BST_Insertion)

/*
 * Ygg's Red-Black Tree, avoiding conditional branches
 */
using InsertYggRBBSTFixtureArith =
    BSTFixture<YggRBTreeInterface<ArithTreeOptions, RBBSTNamerArith>,
               InsertExperiment, BSTInsertOptions>;
BENCHMARK_DEFINE_F(InsertYggRBBSTFixtureArith, BM_BST_Insertion)
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
REGISTER(InsertYggRBBSTFixtureArith, BM_BST_Insertion)

/*
 * Ygg's Weight-Balanced Tree
 */
// Default gamma, delta / twopass
using InsertYggWBDefGDefDTPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTTwopassTreeOptions, WBBSTNamerDefGDefDTP>,
               InsertExperiment, BSTInsertOptions>;
BENCHMARK_DEFINE_F(InsertYggWBDefGDefDTPBSTFixture, BM_BST_Insertion)
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
REGISTER(InsertYggWBDefGDefDTPBSTFixture, BM_BST_Insertion)

// Default gamma, delta / single pass
using InsertYggWBDefGDefDSPBSTFixture = BSTFixture<
    YggWBTreeInterface<WBTSinglepassTreeOptions, WBBSTNamerDefGDefDSP>,
    InsertExperiment, BSTInsertOptions>;
BENCHMARK_DEFINE_F(InsertYggWBDefGDefDSPBSTFixture, BM_BST_Insertion)
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
REGISTER(InsertYggWBDefGDefDSPBSTFixture, BM_BST_Insertion)

// Lai and Wood gamma, delta / single pass
using InsertYggWBLWSPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTSinglepassLWTreeOptions, WBBSTNamerLWSP>,
               InsertExperiment, BSTInsertOptions>;
BENCHMARK_DEFINE_F(InsertYggWBLWSPBSTFixture, BM_BST_Insertion)
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
REGISTER(InsertYggWBLWSPBSTFixture, BM_BST_Insertion)

// Balance-focussed gamma, delta / single pass
using InsertYggWBBalSPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTSinglepassBalTreeOptions, WBBSTNamerBalSP>,
               InsertExperiment, BSTInsertOptions>;
BENCHMARK_DEFINE_F(InsertYggWBBalSPBSTFixture, BM_BST_Insertion)
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
REGISTER(InsertYggWBBalSPBSTFixture, BM_BST_Insertion)

// Balance-focussed gamma, delta / single pass, avoiding conditionals
using InsertYggWBBalSPArithBSTFixture = BSTFixture<
    YggWBTreeInterface<WBTSinglepassBalArithTreeOptions, WBBSTNamerBalSPArith>,
    InsertExperiment, BSTInsertOptions>;
BENCHMARK_DEFINE_F(InsertYggWBBalSPArithBSTFixture, BM_BST_Insertion)
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
REGISTER(InsertYggWBBalSPArithBSTFixture, BM_BST_Insertion)

// integral gamma, delta / single pass
using InsertYggWB3G2DSPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTSinglepass32TreeOptions, WBBSTNamer3G2DSP>,
               InsertExperiment, BSTInsertOptions>;
BENCHMARK_DEFINE_F(InsertYggWB3G2DSPBSTFixture, BM_BST_Insertion)
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
REGISTER(InsertYggWB3G2DSPBSTFixture, BM_BST_Insertion)

// integral gamma, delta / twopass
using InsertYggWB3G2DTPBSTFixture =
    BSTFixture<YggWBTreeInterface<WBTTwopass32TreeOptions, WBBSTNamer3G2DTP>,
               InsertExperiment, BSTInsertOptions>;
BENCHMARK_DEFINE_F(InsertYggWB3G2DTPBSTFixture, BM_BST_Insertion)
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
REGISTER(InsertYggWB3G2DTPBSTFixture, BM_BST_Insertion)

/*
 * Ygg's Energy-Balanced Tree
 */
using InsertYggEBSTFixture =
    BSTFixture<YggEnergyTreeInterface<BasicTreeOptions>, InsertExperiment,
               BSTInsertOptions>;
BENCHMARK_DEFINE_F(InsertYggEBSTFixture, BM_BST_Insertion)
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
REGISTER(InsertYggEBSTFixture, BM_BST_Insertion)

/*
 * Ygg's Zip Tree
 */
using InsertYggZBSTFixture = BSTFixture<YggZTreeInterface<BasicTreeOptions>,
                                        InsertExperiment, BSTInsertOptions>;
BENCHMARK_DEFINE_F(InsertYggZBSTFixture, BM_BST_Insertion)
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
REGISTER(InsertYggZBSTFixture, BM_BST_Insertion)

/*
 * Boost::Intrusive::Set
 */
using InsertBISetBSTFixture =
    BSTFixture<BoostSetInterface, InsertExperiment, BSTInsertOptions>;
BENCHMARK_DEFINE_F(InsertBISetBSTFixture, BM_BST_Insertion)
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
			this->t.erase(n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}
	this->papi.report_and_reset(state);
}
REGISTER(InsertBISetBSTFixture, BM_BST_Insertion)

/*
 * std::set
 */
using InsertStdSetBSTFixture =
    BSTFixture<StdSetInterface, InsertExperiment, BSTInsertOptions>;
BENCHMARK_DEFINE_F(InsertStdSetBSTFixture, BM_BST_Insertion)
(benchmark::State & state)
{
	// Timing is not active outside the for (… : state) loop, thus we don't have
	// to pause it here.
	std::vector<decltype(
	    std::multiset<int>().insert(std::move(this->experiment_nodes[0])))>
	    insertion_iterators;
	insertion_iterators.reserve(this->experiment_nodes.size());

	for (auto _ : state) {
		this->papi.start();
		for (auto & n : this->experiment_nodes) {
			// TODO emplace_back incurs a minimal overhead. Can we work around this?
			insertion_iterators.emplace_back(this->t.insert(std::move(n)));
		}
		this->papi.stop();

		state.PauseTiming();
		// Since we moved, we must completely rebuild the experiment nodes
		this->experiment_nodes.clear();
		for (auto & it : insertion_iterators) {
			this->experiment_nodes.push_back(this->t.extract(it));
		}
		insertion_iterators.clear();
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(InsertStdSetBSTFixture, BM_BST_Insertion)

#ifndef NOMAIN
#include "main.hpp"
#endif

#endif
