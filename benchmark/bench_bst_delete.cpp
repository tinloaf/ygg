#ifndef BENCH_BST_DELETE_HPP
#define BENCH_BST_DELETE_HPP

#include "common_bst.hpp"

/*
 * Ygg's Red-Black Tree
 */
using DeleteYggRBBSTFixture =
	BSTFixture<YggRBTreeInterface<BasicTreeOptions>, false, false, true, false>;
BENCHMARK_DEFINE_F(DeleteYggRBBSTFixture, BM_BST_Deletion)(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi_start();
		for (auto n : this->experiment_node_pointers) {
			this->t.remove(*n);
		}
		this->papi_stop();

		state.PauseTiming();
		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi_report_and_reset(state);
}
BENCHMARK_REGISTER_F(DeleteYggRBBSTFixture, BM_BST_Deletion)
    ->RangeMultiplier(2)
    ->Ranges({{BASE_SIZE, BASE_SIZE * (1 << DOUBLINGS)}, {EXPERIMENT_SIZE, EXPERIMENT_SIZE}});

/*
 * Ygg's Zip Tree
 */
using DeleteYggZBSTFixture =
	BSTFixture<YggZTreeInterface<BasicTreeOptions>, false, false, true, false>;
BENCHMARK_DEFINE_F(DeleteYggZBSTFixture, BM_BST_Deletion)(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi_start();
		for (auto n : this->experiment_node_pointers) {
			this->t.remove(*n);
		}
		this->papi_stop();

		state.PauseTiming();
		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi_report_and_reset(state);
}
BENCHMARK_REGISTER_F(DeleteYggZBSTFixture, BM_BST_Deletion)
    ->RangeMultiplier(2)
    ->Ranges({{BASE_SIZE, BASE_SIZE * (1 << DOUBLINGS)}, {EXPERIMENT_SIZE, EXPERIMENT_SIZE}});

/*
 * Boost::Intrusive::Set
 */
using DeleteBISetBSTFixture =
	BSTFixture<BoostSetInterface, false, false, true, false>;
BENCHMARK_DEFINE_F(DeleteBISetBSTFixture, BM_BST_Deletion)(benchmark::State & state)
{
	for (auto _ : state) {
		this->papi_start();
		for (auto n : this->experiment_node_pointers) {
			this->t.erase(*n);
		}
		this->papi_stop();

		state.PauseTiming();
		for (auto n : this->experiment_node_pointers) {
			this->t.insert(*n);
		}
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi_report_and_reset(state);
}
BENCHMARK_REGISTER_F(DeleteBISetBSTFixture, BM_BST_Deletion)
    ->RangeMultiplier(2)
    ->Ranges({{BASE_SIZE, BASE_SIZE * (1 << DOUBLINGS)}, {EXPERIMENT_SIZE, EXPERIMENT_SIZE}});

/*
 * std::set
 */
using DeleteStdSetBSTFixture =
	BSTFixture<StdSetInterface, false, false, false, false>;
BENCHMARK_DEFINE_F(DeleteStdSetBSTFixture, BM_BST_Deletion)(benchmark::State & state)
{
	std::vector<decltype(this->t)::const_iterator> experiment_iterators;
	std::vector<decltype(std::multiset<int>().extract(0))> extracted_nodes;
	extracted_nodes.reserve(state.range(1));
	std::vector<decltype(this->t)::const_iterator> all_iterators;
	auto it = this->t.begin();
	while (it != this->t.end()) {
		all_iterators.emplace_back(it);
		it++;
	}
	std::sample(all_iterators.begin(), all_iterators.end(),
	            std::back_inserter(experiment_iterators),
	            state.range(1), this->rng);
	all_iterators.clear();
	

	for (auto _ : state) {
		this->papi_start();
		for (auto it : experiment_iterators) {
			extracted_nodes.push_back(std::move(this->t.extract(std::move(it))));
		}
		this->papi_stop();

		state.PauseTiming();
		experiment_iterators.clear();
		for (auto & n : extracted_nodes) {
			experiment_iterators.push_back(this->t.insert(std::move(n)));
		}
		extracted_nodes.clear();
		// TODO shuffling here?
		state.ResumeTiming();
	}

	this->papi_report_and_reset(state);
}
BENCHMARK_REGISTER_F(DeleteStdSetBSTFixture, BM_BST_Deletion)
    ->RangeMultiplier(2)
    ->Ranges({{BASE_SIZE, BASE_SIZE * (1 << DOUBLINGS)}, {EXPERIMENT_SIZE, EXPERIMENT_SIZE}});


#endif
