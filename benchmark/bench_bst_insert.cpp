#ifndef BENCH_BST_INSERT_HPP
#define BENCH_BST_INSERT_HPP

#include "common_bst.hpp"

/*
 * Ygg's Red-Black Tree
 */
using InsertYggRBBSTFixture =
    BSTFixture<YggRBTreeInterface<BasicTreeOptions>, true, false, false, false>;
BENCHMARK_DEFINE_F(InsertYggRBBSTFixture, BM_BST_Insertion)(benchmark::State & state)
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
 * Ygg's Zip Tree
 */
using InsertYggZBSTFixture =
    BSTFixture<YggZTreeInterface<BasicTreeOptions>, true, false, false, false>;
BENCHMARK_DEFINE_F(InsertYggZBSTFixture, BM_BST_Insertion)(benchmark::State & state)
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
REGISTER(InsertYggZBSTFixture, BM_BST_Insertion);

/*
 * Boost::Intrusive::Set
 */
using InsertBISetBSTFixture = BSTFixture<BoostSetInterface, true, false, false, false>;
BENCHMARK_DEFINE_F(InsertBISetBSTFixture, BM_BST_Insertion)(benchmark::State & state)
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
REGISTER(InsertBISetBSTFixture, BM_BST_Insertion);

/*
 * std::set
 */
using InsertStdSetBSTFixture = BSTFixture<StdSetInterface, true, false, false, false>;
BENCHMARK_DEFINE_F(InsertStdSetBSTFixture, BM_BST_Insertion)(benchmark::State & state)
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
			this->experiment_nodes.push_back(std::move(this->t.extract(it)));
		}
		insertion_iterators.clear();
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi.report_and_reset(state);
}
REGISTER(InsertStdSetBSTFixture, BM_BST_Insertion);

#ifndef NOMAIN
#include "main.hpp"
#endif

#endif
