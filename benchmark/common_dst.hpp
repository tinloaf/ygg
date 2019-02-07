#ifndef BENCH_COMMON_DST_HPP
#define BENCH_COMMON_DST_HPP

#include "benchmark.h"
#include <algorithm>
#include <random>
#include <vector>

#include "../src/ygg.hpp"

#include "main.hpp"

// TODO various RBTree / Zip Tree variants!
// TODO make values optional

template <class Interface, bool need_nodes, bool need_values,
          bool need_indices, bool values_from_fixed>
class DSTFixture : public benchmark::Fixture {
public:
	DSTFixture() : rng(std::random_device{}()) {}

	void
	SetUp(const ::benchmark::State & state)
	{
		size_t fixed_count = state.range(0);
		size_t experiment_count = state.range(1);

		std::uniform_int_distribution<> point_distr(
		    std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
		std::uniform_real_distribution<double> val_distr(0, 20);

		this->fixed_nodes.clear();

		for (size_t i = 0; i < fixed_count; ++i) {
			int p1 = point_distr(this->rng);
			int p2 = point_distr(this->rng);
			double value = val_distr(this->rng);

			if (p1 < p2) {
				this->fixed_nodes.push_back(Interface::create_node(p1, p2, value));
				this->fixed_values.push_back({p1, p2, value});
			} else if (p2 < p1) {
				this->fixed_nodes.push_back(Interface::create_node(p2, p1, value));
				this->fixed_values.push_back({p2, p1, value});
			} else {
				this->fixed_nodes.push_back(Interface::create_node(p1, p1 + 1, value));
				this->fixed_values.push_back({p1, p1 + 1, value});
			}
		}
		for (auto & n : this->fixed_nodes) {
			Interface::insert(this->t, n);
		}

		std::vector<std::tuple<int, int, double>> fixed_shuffled;
		if (values_from_fixed) {
			fixed_shuffled.insert(fixed_shuffled.begin(),
			                      this->fixed_values.begin(),
			                      this->fixed_values.end());
			std::shuffle(fixed_shuffled.begin(), fixed_shuffled.end(), this->rng);
		}

		if (need_nodes) {
			this->experiment_nodes.clear();
			for (size_t i = 0; i < experiment_count; ++i) {
				int p1 = point_distr(this->rng);
				int p2 = point_distr(this->rng);
				double value = val_distr(this->rng);

				if (p1 < p2) {
					this->experiment_nodes.push_back(
					    Interface::create_node(p1, p2, value));
				} else if (p2 < p1) {
					this->experiment_nodes.push_back(
					    Interface::create_node(p2, p1, value));
				} else {
					this->experiment_nodes.push_back(
					    Interface::create_node(p1, p1 + 1, value));
				}
			}
		}

		if (need_values) {
			// TODO values-from-fixed is not respected? Is it for the BST?
			for (size_t i = 0; i < experiment_count; ++i) {
				int p1 = point_distr(this->rng);
				int p2 = point_distr(this->rng);
				double value = val_distr(this->rng);

				if (p1 < p2) {
					this->experiment_values.push_back({p1, p2, value});
				} else if (p2 < p1) {
					this->experiment_values.push_back({p2, p1, value});
				} else {
					this->experiment_values.push_back({p1, p1 + 1, value});
				}
			}
		}

		if (need_indices) {
			this->experiment_indices.clear();
			auto range = ygg::utilities::IntegerRange<size_t>(0, fixed_count);
			this->experiment_indices.clear();
			std::sample(range.begin(), range.end(),
			            std::back_inserter(this->experiment_indices),
			            experiment_count,
			            this->rng);
		}
	}

	void
	TearDown(const ::benchmark::State & state)
	{
		Interface::clear(this->t);
	}

	std::vector<typename Interface::Node> fixed_nodes;
	std::vector<std::tuple<int, int, double>> fixed_values;
	
	std::vector<typename Interface::Node> experiment_nodes;
	std::vector<std::tuple<int, int, double>> experiment_values;
	std::vector<size_t> experiment_indices;

	std::mt19937 rng;

	typename Interface::Tree t;
};

// TODO try with max combiner!
using CombinerPack = ygg::EmptyCombinerPack<int, double>;

/*
 * Red-Black DST Interface
 */
template <class MyTreeOptions>
class RBDSTNode : public ygg::DynSegTreeNodeBase<int, double, double,
                                                 CombinerPack, ygg::UseRBTree> {
public:
	int lower;
	int upper;
	double value;
};

template <class MyTreeOptions>
class RBDSTNodeTraits
    : public ygg::DynSegTreeNodeTraits<RBDSTNode<MyTreeOptions>> {
public:
	using Node = RBDSTNode<MyTreeOptions>;

	static int
	get_lower(const Node & n)
	{
		return n.lower;
	}
	static int
	get_upper(const Node & n)
	{
		return n.upper;
	}
	static double
	get_value(const Node & n)
	{
		return n.value;
	}
};

template <class MyTreeOptions>
class RBDSTInterface {
public:
	using Node = RBDSTNode<MyTreeOptions>;
	using Tree =
	    ygg::DynamicSegmentTree<Node, RBDSTNodeTraits<MyTreeOptions>,
	                            CombinerPack, MyTreeOptions, ygg::UseRBTree>;

	static void
	insert(Tree & t, Node & n)
	{
		t.insert(n);
	}

	static Node
	create_node(int lower, int upper, double val)
	{
		Node n;
		n.lower = lower;
		n.upper = upper;
		n.value = val;

		return n;
	}

	static void
	clear(Tree & t)
	{
		t.clear();
	}
};


/*
 * Zipping DST Interface
 */
template <class MyTreeOptions>
class ZDSTNode : public ygg::DynSegTreeNodeBase<int, double, double,
                                                 CombinerPack, ygg::UseZipTree> {
public:
	int lower;
	int upper;
	double value;
};

template <class MyTreeOptions>
class ZDSTNodeTraits
    : public ygg::DynSegTreeNodeTraits<ZDSTNode<MyTreeOptions>> {
public:
	using Node = ZDSTNode<MyTreeOptions>;

	static int
	get_lower(const Node & n)
	{
		return n.lower;
	}
	static int
	get_upper(const Node & n)
	{
		return n.upper;
	}
	static double
	get_value(const Node & n)
	{
		return n.value;
	}
};

template <class MyTreeOptions>
class ZDSTInterface {
public:
	using Node = ZDSTNode<MyTreeOptions>;
	using Tree =
	    ygg::DynamicSegmentTree<Node, ZDSTNodeTraits<MyTreeOptions>,
	                            CombinerPack, MyTreeOptions, ygg::UseZipTree>;

	static void
	insert(Tree & t, Node & n)
	{
		t.insert(n);
	}

	static Node
	create_node(int lower, int upper, double val)
	{
		Node n;
		n.lower = lower;
		n.upper = upper;
		n.value = val;

		return n;
	}

	static void
	clear(Tree & t)
	{
		t.clear();
	}
};


using BasicDSTTreeOptions =
    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE, ygg::TreeFlags::ZTREE_USE_HASH,
                     ygg::TreeFlags::ZTREE_RANK_TYPE<uint8_t>,
                     ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT<
                         9859957398433823229ul>,
                     ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_MODUL<
                         std::numeric_limits<size_t>::max()>>;

#endif
