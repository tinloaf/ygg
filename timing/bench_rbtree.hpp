//
// Created by lukas on 01.09.17.
//

#ifndef YGG_BENCH_RBTREE_HPP
#define YGG_BENCH_RBTREE_HPP

#include <celero/Celero.h>

#include "../src/ygg.hpp"
#include <boost/intrusive/set.hpp>

using namespace ygg;

class RBTreeBaseFixture : public celero::TestFixture {
public:
	RBTreeBaseFixture() {
		const int totalNumberOfTests = 8;

		this->instance_sizes.clear();
		for(int i = 0; i < totalNumberOfTests; i++)
		{
			this->instance_sizes.push_back({static_cast<int32_t>(std::pow(2, (i+6)+1)), 0u});
		}
	}

	virtual std::vector<std::pair<int64_t, uint64_t>> getExperimentValues() const override
	{
		return this->instance_sizes;
	};

	virtual void setUp(const int64_t value_count_in) override
	{
		std::set<int> seen;

		this->value_count = value_count_in;
		this->values.resize(this->value_count);
		for (int i = 0 ; i < this->value_count ; ++i) {
			int val = rand();

			while (seen.find(val) != seen.end()) {
				val = rand();
			}

			this->values[i] = val;
			seen.insert(val);
		}
	}

	virtual void tearDown() override
	{
		this->values.clear();
	}

	std::vector<std::pair<int64_t, uint64_t >> instance_sizes;

	std::vector<int> values;
	int64_t value_count;
};

/*
 * Ygg fixtures
 */
class YggTreeBaseFixture : public RBTreeBaseFixture {
public:

	class Node : public RBTreeNodeBase<Node, TreeOptions<>>
	{
	public:
		int value;

		bool operator<(const Node & rhs) const {
			return this->value < rhs.value;
		}
	};

	using Tree = RBTree<Node, RBDefaultNodeTraits<Node>, TreeOptions<>>;

	virtual void setUp(const int64_t value_count) override
	{
		this->RBTreeBaseFixture::setUp(value_count);

		this->nodes.resize(value_count);
		for (int i = 0 ; i < value_count ; ++i) {
			this->nodes[i].value = this->values[i];
		}
	}

	virtual void tearDown() override
	{
		this->RBTreeBaseFixture::tearDown();

		this->nodes.clear();
	}

	std::vector<Node> nodes;
	Tree t;
};

class YggTreeInsertFixture : public YggTreeBaseFixture
{};


class YggTreeSearchFixture : public YggTreeBaseFixture
{
public:
	virtual void setUp(const int64_t value_count) override
	{
		this->YggTreeBaseFixture::setUp(value_count);

		for (auto & n : this->nodes) {
			this->t.insert(n);
			this->search_values.push_back(&n);
		}

		std::random_shuffle(this->search_values.begin(), this->search_values.end());
	}

	virtual void tearDown() override
	{
		this->t.clear();
		this->search_values.clear();

		this->YggTreeBaseFixture::tearDown();
	}

	std::vector<Node *> search_values;
};

/*
 * Boost fixtures
 */

class BoostSetBaseFixture : public RBTreeBaseFixture
{
public:
	class Node : public boost::intrusive::set_base_hook<boost::intrusive::link_mode<boost
	                                                                                ::intrusive::link_mode_type::normal_link>> {
	public:
		int value;

		bool operator<(const Node & rhs) const {
			return this->value < rhs.value;
		}
	};

	using Tree = boost::intrusive::set<Node>;

	virtual void setUp(const int64_t value_count) override
	{
		this->RBTreeBaseFixture::setUp(value_count);

		this->nodes.resize(value_count);
		for (int i = 0 ; i < value_count ; ++i) {
			this->nodes[i].value = this->values[i];
		}
	}

	virtual void tearDown() override
	{
		this->RBTreeBaseFixture::tearDown();

		this->nodes.clear();
	}

	std::vector<Node> nodes;
	Tree t;
};

class BoostSetInsertFixture : public BoostSetBaseFixture
{};

class BoostSetSearchFixture : public BoostSetBaseFixture
{
public:
	virtual void setUp(const int64_t value_count) override
	{
		this->BoostSetBaseFixture::setUp(value_count);

		for (auto & n : this->nodes) {
			this->t.insert(n);
			this->search_values.push_back(&n);
		}

		std::random_shuffle(this->search_values.begin(), this->search_values.end());
	}

	virtual void tearDown() override
	{
		this->t.clear();
		this->search_values.clear();

		this->BoostSetBaseFixture::tearDown();
	}

	std::vector<Node *> search_values;
};

/*
 * Actual tests
 */

BENCHMARK_F(RBTreeInsert, Ygg, YggTreeInsertFixture, 30, 50)
{
	this->t.clear();

	for (auto & n : this->nodes) {
		this->t.insert(n);
	}
	celero::DoNotOptimizeAway(this->t);
}


BASELINE_F(RBTreeInsert, BoostMultiset, BoostSetInsertFixture, 30, 50)
{
	this->t.clear();

	for (auto & n : this->nodes) {
		this->t.insert(n);
	}
	celero::DoNotOptimizeAway(this->t);
}

BENCHMARK_F(RBTreeSearch, Ygg, YggTreeSearchFixture, 30, 50)
{
  int sum = 0;
	for (auto & v : this->search_values) {
		auto it = this->t.find(*v);
		sum += it->value;
	}

	celero::DoNotOptimizeAway(sum);
}



BASELINE_F(RBTreeSearch, BoostSet, BoostSetSearchFixture, 30, 50)
{
	int sum = 0;
		for (auto & v : this->search_values) {
			auto it = this->t.find(*v);
			sum += it->value;
		}

	celero::DoNotOptimizeAway(sum);
}

BENCHMARK_F(RBTreeIteration, Ygg, YggTreeSearchFixture, 50, 200)
{
	for (const auto & n : this->t) {
		//sum += n.value;
		celero::DoNotOptimizeAway(n);
	}
}

BASELINE_F(RBTreeIteration, BoostSet, BoostSetSearchFixture, 50, 200)
{
	for (const auto &n : this->t) {
		//sum += n.value;
		celero::DoNotOptimizeAway(n);
	}
}

BENCHMARK_F(RBTreeDelete, Ygg, YggTreeSearchFixture, 1000, 1)
{
	for (auto & v : this->search_values) {
		this->t.remove(*v);
	}

	celero::DoNotOptimizeAway(this->t);
}

BASELINE_F(RBTreeDelete, BoostSet, BoostSetSearchFixture, 1000, 1)
{
	for (auto & v : this->search_values) {
		this->t.erase(*v);
	}

	celero::DoNotOptimizeAway(this->t);
}

#endif //YGG_BENCH_RBTREE_HPP
