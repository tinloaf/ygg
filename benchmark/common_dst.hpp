#ifndef BENCH_COMMON_DST_HPP
#define BENCH_COMMON_DST_HPP

#include "benchmark.h"
#include <algorithm>
#include <random>
#include <vector>

#include "../src/ygg.hpp"

// TODO various RBTree / Zip Tree variants!
// TODO make values optional

template <class Interface, bool need_nodes, bool need_values,
          bool need_node_pointers, bool values_from_fixed>
class Fixture : public benchmark::Fixture {
public:
	Fixture() : rng(std::random_device{}()) {}

	void
	SetUp(const ::benchmark::State & state)
	{
		size_t fixed_count = state.range(0);
		size_t experiment_count = state.range(1);

		std::uniform_int_distribution<> point_distr(
		    std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
		std::uniform_real_distribution<double> val_distr(0, 20);

		std::vector<std::tuple<int, int, double>> fixed_values;

		this->fixed_nodes.clear();

		for (size_t i = 0; i < fixed_count; ++i) {
			int p1 = point_distr(this->rng);
			int p2 = point_distr(this->rng);
			double value = val_distr(this->rng);

			if (p1 < p2) {
				this->fixed_nodes.push_back(Interface::create_node(p1, p2, value));
				if (values_from_fixed) {
					fixed_values.push_back({p1, p2, value});
				}
			} else if (p2 < p1) {
				this->fixed_nodes.push_back(Interface::create_node(p2, p1, value));
				if (values_from_fixed) {
					fixed_values.push_back({p2, p1, value});
				}
			} else {
				this->fixed_nodes.push_back(Interface::create_node(p1, p1 + 1, value));
				if (values_from_fixed) {
					fixed_values.push_back({p1, p1 + 1, value});
				}
			}
		}
		for (auto & n : this->fixed_nodes) {
			Interface::insert(this->t, n);
		}

		if (values_from_fixed) {
			std::shuffle(fixed_values.begin(), fixed_values.end(), this->rng);
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

		if (need_node_pointers) {
			this->experiment_node_pointers.clear();
			std::vector<typename Interface::Node *> ptrs;
			for (typename Interface::Node & n : this->fixed_nodes) {
				ptrs.push_back(&n);
			}
			std::shuffle(ptrs.begin(), ptrs.end(), this->rng);
			this->experiment_node_pointers.insert(
			    this->experiment_node_pointers.begin(), ptrs.begin(),
			    ptrs.begin() + experiment_count);
		}
	}

	void
	TearDown(const ::benchmark::State & state)
	{
		Interface::clear(this->t);
	}

	std::vector<typename Interface::Node> fixed_nodes;

	std::vector<typename Interface::Node> experiment_nodes;
	std::vector<std::tuple<int, int, double>> experiment_values;
	std::vector<typename Interface::Node *> experiment_node_pointers;

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

using BasicTreeOptions =
    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE, ygg::TreeFlags::ZTREE_USE_HASH,
                     ygg::TreeFlags::ZTREE_RANK_TYPE<uint8_t>,
                     ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT<
                         9859957398433823229ul>,
                     ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_MODUL<
                         std::numeric_limits<size_t>::max()>>;

#endif
