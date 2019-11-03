#ifndef BENCH_COMMON_DST_HPP
#define BENCH_COMMON_DST_HPP

#include "../src/ygg.hpp"
#include "benchmark.h"
#include "common.hpp"

#include <algorithm>
#include <draup.hpp>
#include <random>
#include <vector>

// TODO various RBTree / Zip Tree variants!
// TODO make values optional

template <class Interface, typename Experiment, bool need_nodes,
          bool need_values, bool need_indices, bool values_from_fixed>
class DSTFixture : public benchmark::Fixture {
public:
	constexpr static bool has_node_interface = false;

	DSTFixture() : rng(std::random_device{}()) {}

	static std::string
	get_name()
	{
		auto experiment_c = Experiment{};
		std::string name = std::string("DST :: ") +
		                   boost::hana::to<char const *>(experiment_c) +
		                   std::string(" :: ") + Interface::get_name();
		return name;
	}

	void
	set_name(std::string name)
	{
		this->SetName(name.c_str());
	}

	void
	SetUp(const ::benchmark::State & state)
	{
		this->papi.initialize();

		size_t fixed_count = static_cast<size_t>(state.range(0));
		size_t experiment_count = static_cast<size_t>(state.range(1));
		int seed = static_cast<int>(state.range(2));
		this->rng = std::mt19937(static_cast<unsigned long>(seed));

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
			fixed_shuffled.insert(fixed_shuffled.begin(), this->fixed_values.begin(),
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
			this->experiment_values.clear();
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
			            experiment_count, this->rng);
		}
	}

	void
	TearDown(const ::benchmark::State & state)
	{
		(void)state;
		Interface::clear(this->t);
	}

	std::vector<typename Interface::Node> fixed_nodes;
	std::vector<std::tuple<int, int, double>> fixed_values;

	std::vector<typename Interface::Node> experiment_nodes;
	std::vector<std::tuple<int, int, double>> experiment_values;
	std::vector<size_t> experiment_indices;

	std::mt19937 rng;

	typename Interface::Tree t;

	PapiMeasurements papi;
};

// TODO try with max combiner!
using CombinerPack = ygg::EmptyCombinerPack<int, double>;

/*
 * Red-Black DST Interface
 */
template <class MyTreeOptions>
class RBDSTNode
    : public ygg::DynSegTreeNodeBase<int, double, double, CombinerPack,
                                     ygg::UseDefaultRBTree> {
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
	using Tree = ygg::DynamicSegmentTree<Node, RBDSTNodeTraits<MyTreeOptions>,
	                                     CombinerPack, MyTreeOptions,
	                                     ygg::UseDefaultRBTree>;

	static std::string
	get_name()
	{
		return "RBTree";
	}

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
 * Weight-Balanced DST Interface
 */
template <class MyTreeOptions, class... UnderlyingOptions>
class WBDSTNode
    : public ygg::DynSegTreeNodeBase<int, double, double, CombinerPack,
                                     ygg::UseWBTree<UnderlyingOptions...>> {
public:
	int lower;
	int upper;
	double value;
};

template <class MyTreeOptions, class... UnderlyingOptions>
class WBDSTNodeTraits : public ygg::DynSegTreeNodeTraits<
                            WBDSTNode<MyTreeOptions, UnderlyingOptions...>> {
public:
	using Node = WBDSTNode<MyTreeOptions, UnderlyingOptions...>;

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

template <class MyTreeOptions, class... UnderlyingOptions>
class WBDSTInterface {
public:
	using Node = WBDSTNode<MyTreeOptions, UnderlyingOptions...>;
	using Tree = ygg::DynamicSegmentTree<
	    Node, WBDSTNodeTraits<MyTreeOptions, UnderlyingOptions...>, CombinerPack,
	    MyTreeOptions, ygg::UseWBTree<UnderlyingOptions...>>;

	static std::string
	get_name()
	{
		using DummyOptions = ygg::TreeOptions<UnderlyingOptions...>;
		std::string sp_tp;
		if constexpr (DummyOptions::wbt_single_pass) {
			sp_tp = "SP";
		} else {
			sp_tp = "TP";
		}

		return std::string("WBTree[") + DummyOptions::wbt_delta_str() +
		       std::string(",") + DummyOptions::wbt_gamma_str() + std::string(",") +
		       sp_tp + std::string("]");
	}

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
class ZDSTNode
    : public ygg::DynSegTreeNodeBase<int, double, double, CombinerPack,
                                     ygg::UseDefaultZipTree> {
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
	    ygg::DynamicSegmentTree<Node, ZDSTNodeTraits<MyTreeOptions>, CombinerPack,
	                            MyTreeOptions, ygg::UseDefaultZipTree>;

	static std::string
	get_name()
	{
		return "ZipTree";
	}

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
