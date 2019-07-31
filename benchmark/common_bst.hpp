#ifndef BENCH_COMMON_BST_HPP
#define BENCH_COMMON_BST_HPP

#include "../src/ygg.hpp"
#include "benchmark.h"
#include "common.hpp"
#include "random.hpp"

#include <algorithm>
#include <boost/intrusive/set.hpp>
#include <cstdlib>
#include <draup.hpp>
#include <functional>
#include <limits>
#include <memory>
#include <random>
#include <unordered_set>
#include <vector>

#ifdef USEPAPI
#include <papi.h>
#endif

#ifdef USEZIPF
#define DYN_GENERATOR UseZipf
#else
#ifdef USESKEWED
#define DYN_GENERATOR UseSkewed
#else
#define DYN_GENERATOR UseUniform
#endif
#endif

struct WBBSTNamerDefGDefDTP
{
	constexpr static const char * name = "1+sqrt(2),sqrt(2),TP";
};
struct WBBSTNamerDefGDefDSP
{
	constexpr static const char * name = "1+sqrt(2),sqrt(2),SP";
};
struct WBBSTNamerBalSP
{
	constexpr static const char * name = "2,3/2,SP(opt)";
};
struct WBBSTNamerBalSPArith
{
	constexpr static const char * name = "2,3/2,SP(opt),arith";
};
struct WBBSTNamerLWSP
{
	constexpr static const char * name = "3,4/3,SP(opt)";
};
struct WBBSTNamer3G2DSP
{
	constexpr static const char * name = "3,2,SP";
};
struct WBBSTNamer3G2DTP
{
	constexpr static const char * name = "3,2,TP";
};

struct WBBSTNamerDefGDefDSPOPT
{
	constexpr static const char * name = "1+sqrt(2),sqrt(2),SP,opt";
};
struct WBBSTNamer3G2DSPOPT
{
	constexpr static const char * name = "3,2,SP,opt";
};
struct RBBSTNamerDefault
{
	constexpr static const char * name = "RBTree ";
};
struct RBBSTNamerArith
{
	constexpr static const char * name = "RBTree[arith] ";
};

struct UseNone
{
	static constexpr bool enable = false;
};

struct UseUniform
{
	static constexpr bool enable = true;
	using Randomizer = UniformDistr;
	static constexpr int min = std::numeric_limits<int>::min();
	static constexpr int max = std::numeric_limits<int>::max();

	static Randomizer
	create(long unsigned int seed)
	{
		return UniformDistr(seed);
	}
};

struct UseZipf
{
	static constexpr bool enable = true;
	using Randomizer = ZipfDistr;
	static constexpr int min = std::numeric_limits<int>::min();
	static constexpr int max = 10000; // TODO is this sane?

	static Randomizer
	create(long unsigned int seed)
	{
		return ZipfDistr(seed, 1.0);
	}
};

struct UseSkewed
{
	static constexpr bool enable = true;
	using Randomizer = MaekinenSkewedDistr;
	static constexpr int min = std::numeric_limits<int>::min() + 1000;
	static constexpr int max =
	    static_cast<int>(std::numeric_limits<int>::max() * 0.8);

	static Randomizer
	create(long unsigned int seed)
	{
		return MaekinenSkewedDistr(seed, 3, 1000);
	}
};

template <class T, class Compare = decltype(std::less<T>{})>
void
presort(std::vector<T> & v, size_t shuffle_count, size_t seed,
        Compare cmp = std::less<T>{})
{
	std::mt19937 rng(seed);

	std::sort(v.begin(), v.end(), cmp);

	std::vector<size_t> indices(v.size());
	std::iota(indices.begin(), indices.end(), size_t{0});
	std::shuffle(indices.begin(), indices.end(), rng);

	T last_element = std::move(v[indices[shuffle_count - 1]]);
	for (size_t i = 1; i < shuffle_count; ++i) {
		v[indices[i]] = std::move(v[indices[i - 1]]);
	}
	v[0] = std::move(last_element);
};

struct DefaultBenchmarkOptions
{
	constexpr static bool distinct = false;
	constexpr static bool fixed_presort = false;
	constexpr static bool values_from_fixed = false;
	constexpr static bool need_nodes = false;
	constexpr static bool nodes_presort = false;
	constexpr static bool need_node_pointers = false;
	constexpr static bool pointers_presort = false;
	constexpr static bool need_values = false;
	constexpr static bool values_presort = false;

	constexpr static size_t node_value_change_percentage = 0;
};

template <class Interface, class Experiment, class Options>
class BSTFixture : public benchmark::Fixture {
public:
	using NodeInterface = Interface;

	static std::string
	get_name()
	{
		auto experiment_c = Experiment{};
		std::string name = std::string("BST :: ") +
		                   boost::hana::to<char const *>(experiment_c) +
		                   std::string(" :: ") + Interface::get_name();
		return name;
	}

	void
	set_name(std::string name)
	{
		this->SetName(name.c_str());
	}

	BSTFixture() = default;

	void
	SetUp(const ::benchmark::State & state)
	{
		Interface::clear(this->t);
		this->papi.initialize();

		size_t fixed_count = static_cast<size_t>(state.range(0));
		size_t experiment_count = static_cast<size_t>(state.range(1));
		int seed = static_cast<int>(state.range(2));
		this->rng = std::mt19937(static_cast<unsigned long>(seed));

		auto main_rnd =
		    Options::MainRandomizer::create(static_cast<unsigned long>(seed));

		this->fixed_nodes.clear();
		this->fixed_values.clear();

		std::unordered_set<int> seen_values;
		for (size_t i = 0; i < fixed_count; ++i) {
			int val = main_rnd.generate(Options::MainRandomizer::min,
			                            Options::MainRandomizer::max);
			if (Options::distinct) {
				while (seen_values.find(val) != seen_values.end()) {
					val = main_rnd.generate(Options::MainRandomizer::min,
					                        Options::MainRandomizer::max);
				}
				seen_values.insert(val);
			}

			this->fixed_values.push_back(val);
		}

		if constexpr (Options::fixed_presort) {
			size_t presort_count =
			    this->fixed_values.size() * Options::fixed_presort_fraction;
			presort(this->fixed_values, presort_count, this->rng());
		}

		for (int val : this->fixed_values) {
			this->fixed_nodes.push_back(Interface::create_node(val));
		}
		// DO NOT MERGE THESE LOOPS
		for (auto & n : this->fixed_nodes) {
			Interface::insert(this->t, n);
		}

		std::vector<int> shuffled_values;
		if (Options::values_from_fixed) {
			shuffled_values.insert(shuffled_values.begin(),
			                       this->fixed_values.begin(),
			                       this->fixed_values.end());
			std::shuffle(shuffled_values.begin(), shuffled_values.end(), this->rng);
		}

		if constexpr (Options::need_nodes) {
			seen_values.clear();
			auto rnd = Options::NodeRandomizer::create(rng());

			this->experiment_nodes.clear();
			for (size_t i = 0; i < experiment_count; ++i) {
				int val;

				if (Options::values_from_fixed) {
					size_t rand_index = static_cast<size_t>(
					    rnd.generate(0, static_cast<int>(this->fixed_values.size())));
					val = fixed_values[rand_index];
				} else {
					val = rnd.generate(Options::NodeRandomizer::min,
					                   Options::NodeRandomizer::max);
					if (Options::distinct) {
						while (seen_values.find(val) != seen_values.end()) {
							val = rnd.generate(Options::NodeRandomizer::min,
							                   Options::NodeRandomizer::max);
						}
						seen_values.insert(val);
					}
				}

				this->experiment_nodes.push_back(Interface::create_node(val));
			}
			if constexpr (Options::nodes_presort) {
				size_t presort_count =
				    this->experiment_nodes.size() * Options::nodes_presort_fraction;
				presort(this->experiment_nodes, presort_count, this->rng(),
				        [](const typename Interface::Node & lhs,
				           const typename Interface::Node & rhs) {
					        return Interface::get_value(lhs) < Interface::get_value(rhs);
				        });
			}
		}

		if constexpr (Options::need_node_pointers) {
			auto rnd = Options::NodePointerRandomizer::create(rng());
			std::unordered_set<typename Interface::Node *> seen_nodes;
			this->experiment_node_pointers.clear();
			for (size_t i = 0; i < experiment_count; ++i) {
				size_t rnd_index = static_cast<size_t>(
				    rnd.generate(0, static_cast<int>(this->fixed_nodes.size())));
				auto node_ptr = &this->fixed_nodes[rnd_index];
				if (Options::distinct) {
					while (seen_nodes.find(node_ptr) != seen_nodes.end()) {
						rnd_index = static_cast<size_t>(
						    rnd.generate(0, static_cast<int>(this->fixed_nodes.size())));
						assert(rnd_index < this->fixed_nodes.size());
						node_ptr = &this->fixed_nodes[rnd_index];
					}
					seen_nodes.insert(node_ptr);
				}
				this->experiment_node_pointers.push_back(&this->fixed_nodes[rnd_index]);
			}

			if constexpr (Options::pointers_presort) {
				size_t presort_count = this->experiment_node_pointers.size() *
				                       Options::pointers_presort_fraction;
				presort(this->experiment_node_pointers, presort_count, this->rng(),
				        [](const typename Interface::Node * lhs,
				           const typename Interface::Node * rhs) {
					        return Interface::get_value(*lhs) <
					               Interface::get_value(*rhs);
				        });
			}
		}

		if constexpr (Options::need_values) {
			seen_values.clear();
			auto rnd = Options::ValueRandomizer::create(rng());

			this->experiment_values.clear();
			for (size_t i = 0; i < experiment_count; ++i) {
				int val;

				if (Options::values_from_fixed) {
					size_t rnd_index = static_cast<size_t>(
					    rnd.generate(0, static_cast<int>(this->fixed_values.size())));
					val = this->fixed_values[rnd_index];
				} else {
					int min = Options::ValueRandomizer::min;
					int max = Options::ValueRandomizer::max;

					if constexpr (Options::node_value_change_percentage > 0) {
						double node_value_change =
						    static_cast<double>(Options::node_value_change_percentage) /
						    100.0;
						min = static_cast<int>(std::round(
						    Interface::get_value(*this->experiment_node_pointers[i]) *
						    (1 - node_value_change)));
						if (std::numeric_limits<double>::max() / (1 + node_value_change) >
						    Interface::get_value(*this->experiment_node_pointers[i])) {
							max = static_cast<int>(std::round(
							    Interface::get_value(*this->experiment_node_pointers[i]) *
							    (1 + node_value_change)));
						}

						if (min > max) {
							// Negative values
							std::swap(min, max);
						}
					}

					val = rnd.generate(min, max);
					if (Options::distinct) {
						while (seen_values.find(val) != seen_values.end()) {
							val = rnd.generate(min, max);
						}
						seen_values.insert(val);
					}
				}

				this->experiment_values.push_back(val);
			}
			if constexpr (Options::values_presort) {
				size_t presort_count =
				    this->experiment_values.size() * Options::values_presort_fraction;
				presort(this->fixed_values, presort_count, this->rng());
			}
		}
	}

	void
	TearDown(const ::benchmark::State & state)
	{
		(void)state;
		Interface::clear(this->t);
	}

	std::vector<int> fixed_values;
	std::vector<typename Interface::Node> fixed_nodes;

	std::vector<typename Interface::Node> experiment_nodes;
	std::vector<int> experiment_values;
	std::vector<typename Interface::Node *> experiment_node_pointers;

	typename Interface::Tree t;

	PapiMeasurements papi;
	std::mt19937 rng;
};

/*
 * Red-Black Tree Interface
 */
template <class MyTreeOptions>
class RBNode
    : public ygg::RBTreeNodeBase<RBNode<MyTreeOptions>, MyTreeOptions> {
private:
	int value;

public:
	RBNode(int value_in) : value(value_in){};

	void
	set_value(int new_value)
	{
		this->value = new_value;
	}

	int
	get_value() const
	{
		return this->value;
	}

	bool
	operator<(const RBNode<MyTreeOptions> & rhs) const
	{
		return this->value < rhs.value;
	}
};

template <class T>
bool
operator<(const RBNode<T> & lhs, int rhs)
{
	return lhs.get_value() < rhs;
}
template <class T>
bool
operator<(int lhs, const RBNode<T> & rhs)
{
	return lhs < rhs.get_value();
}

template <class MyTreeOptions, class Namer = RBBSTNamerDefault>
class YggRBTreeInterface {
public:
	using Node = RBNode<MyTreeOptions>;
	using Tree = ygg::RBTree<Node, ygg::RBDefaultNodeTraits, MyTreeOptions>;

	static void
	insert(Tree & t, Node & n)
	{
		t.insert(n);
	}

	static std::string
	get_name()
	{
		return Namer::name;
	}

	static int
	get_value(const Node & n)
	{
		return n.get_value();
	}

	static void
	set_value(Node & n, int val)
	{
		n.set_value(val);
	}

	static Node
	create_node(int val)
	{
		return Node(val);
	}

	static void
	clear(Tree & t)
	{
		t.clear();
	}
};

/*
 * Weight-Balanced Tree Interface
 */
template <class MyTreeOptions>
class WBNode
    : public ygg::WBTreeNodeBase<WBNode<MyTreeOptions>, MyTreeOptions> {
private:
	int value;

public:
	WBNode(int value_in) : value(value_in){};

	void
	set_value(int new_value)
	{
		this->value = new_value;
	}

	int
	get_value() const
	{
		return this->value;
	}

	bool
	operator<(const WBNode<MyTreeOptions> & rhs) const
	{
		return this->value < rhs.value;
	}
};

template <class T>
bool
operator<(const WBNode<T> & lhs, int rhs)
{
	return lhs.get_value() < rhs;
}
template <class T>
bool
operator<(int lhs, const WBNode<T> & rhs)
{
	return lhs < rhs.get_value();
}

template <class MyTreeOptions, class BenchmarkNamer>
class YggWBTreeInterface {
public:
	using Node = WBNode<MyTreeOptions>;
	using Tree = ygg::WBTree<Node, ygg::WBDefaultNodeTraits, MyTreeOptions>;

	static void
	insert(Tree & t, Node & n)
	{
		t.insert(n);
	}

	static std::string
	get_name()
	{
		std::ostringstream name;
		name << "WBTree[" << BenchmarkNamer::name << "] ";
		return name.str();
	}

	static int
	get_value(const Node & n)
	{
		return n.get_value();
	}

	static void
	set_value(Node & n, int val)
	{
		n.set_value(val);
	}

	static Node
	create_node(int val)
	{
		return Node(val);
	}

	static void
	clear(Tree & t)
	{
		t.clear();
	}
};

/*
 * Energy-Balanced Tree Interface
 */
template <class MyTreeOptions>
class ENode
    : public ygg::EnergyTreeNodeBase<ENode<MyTreeOptions>, MyTreeOptions> {
private:
	int value;

public:
	ENode(int value_in) : value(value_in){};

	void
	set_value(int new_value)
	{
		this->value = new_value;
	}

	int
	get_value() const
	{
		return this->value;
	}

	bool
	operator<(const ENode<MyTreeOptions> & rhs) const
	{
		return this->value < rhs.value;
	}
};

template <class T>
bool
operator<(const ENode<T> & lhs, int rhs)
{
	return lhs.get_value() < rhs;
}
template <class T>
bool
operator<(int lhs, const ENode<T> & rhs)
{
	return lhs < rhs.get_value();
}

template <class MyTreeOptions>
class YggEnergyTreeInterface {
public:
	using Node = ENode<MyTreeOptions>;
	using Tree = ygg::EnergyTree<Node, MyTreeOptions>;

	static void
	insert(Tree & t, Node & n)
	{
		t.insert(n);
	}

	static std::string
	get_name()
	{
		return "EnergyTree";
	}

	static int
	get_value(const Node & n)
	{
		return n.get_value();
	}

	static void
	set_value(Node & n, int val)
	{
		n.set_value(val);
	}

	static Node
	create_node(int val)
	{
		return Node(val);
	}

	static void
	clear(Tree & t)
	{
		t.clear();
	}
};

/*
 * Zip Tree Interface
 */
template <class MyTreeOptions>
class ZipNode
    : public ygg::ZTreeNodeBase<ZipNode<MyTreeOptions>, MyTreeOptions> {
private:
	int value;

public:
	ZipNode(int value_in) : value(value_in) { this->update_rank(); }

	void
	set_value(int new_value)
	{
		this->value = new_value;
		this->update_rank();
	}

	int
	get_value() const
	{
		return this->value;
	}

	bool
	operator<(const ZipNode<MyTreeOptions> & rhs) const
	{
		return this->value < rhs.value;
	}
};
template <class T>
bool
operator<(const ZipNode<T> & lhs, int rhs)
{
	return lhs.get_value() < rhs;
}
template <class T>
bool
operator<(int lhs, const ZipNode<T> & rhs)
{
	return lhs < rhs.get_value();
}

template <class MyTreeOptions>
class YggZTreeInterface {
public:
	using Node = ZipNode<MyTreeOptions>;

	using Tree =
	    ygg::ZTree<Node, ygg::ZTreeDefaultNodeTraits<Node>, MyTreeOptions>;

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

	static int
	get_value(const Node & n)
	{
		return n.get_value();
	}

	static Node
	create_node(int val)
	{
		return Node(val);
	}

	static void
	clear(Tree & t)
	{
		t.clear();
	}
};
// Necessary for rank-by-hash
namespace std {
template <class T>
struct hash<ZipNode<T>>
{
	size_t
	operator()(const ZipNode<T> & n) const
	{
		return static_cast<size_t>(n.get_value());
	}
};
} // namespace std

/*
 * Boost::Intrusive::Set Interface
 */
class BoostSetInterface {
public:
	class Node
	    : public boost::intrusive::set_base_hook<boost::intrusive::link_mode<
	          boost::intrusive::link_mode_type::normal_link>> {
	public:
		int value;

		Node(int value_in) : value(value_in) {}

		bool
		operator<(const Node & rhs) const
		{
			return this->value < rhs.value;
		}
	};

	using Tree = boost::intrusive::multiset<Node>;

	static std::string
	get_name()
	{
		return "boost::intrusive::multiset";
	}

	static int
	get_value(const Node & n)
	{
		return n.value;
	}

	static void
	set_value(Node & n, int val)
	{
		n.value = val;
	}

	static void
	insert(Tree & t, Node & n)
	{
		t.insert(n);
	}

	static Node
	create_node(int val)
	{
		return Node(val);
	}

	static void
	clear(Tree & t)
	{
		t.clear();
	}
};

/*
 * Std::Set's interface
 */
class StdSetInterface {
public:
	using Node = decltype(std::multiset<int>().extract(0));
	using Tree = std::multiset<int>;

	static std::string
	get_name()
	{
		return "std::multiset";
	}

	static void
	insert(Tree & t, Node & n)
	{
		// TODO this is very unclean
		t.insert(std::move(n));
	}

	static int
	get_value(const Node & n)
	{
		return n.value();
	}

	static void
	insert(Tree & t, Node && n)
	{
		// TODO this is very unclean
		t.insert(std::move(n));
	}

	static Node
	create_node(int val)
	{
		std::multiset<int> donor;
		donor.insert(val);

		return donor.extract(donor.begin());
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

using ArithTreeOptions =
    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE, ygg::TreeFlags::ZTREE_USE_HASH,
                     ygg::TreeFlags::ZTREE_RANK_TYPE<uint8_t>,
                     ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT<
                         9859957398433823229ul>,
                     ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_MODUL<
                         std::numeric_limits<size_t>::max()>,
                     ygg::TreeFlags::MICRO_AVOID_CONDITIONALS>;

/* Variants of the weight-balanced tree */
using WBTTwopassTreeOptions = ygg::TreeOptions<ygg::TreeFlags::MULTIPLE>;
using WBTSinglepassTreeOptions =
    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE, ygg::TreeFlags::WBT_SINGLE_PASS>;

using WBTTwopass32TreeOptions =
    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE,
                     ygg::TreeFlags::WBT_DELTA_NUMERATOR<3>,
                     ygg::TreeFlags::WBT_DELTA_DENOMINATOR<1>,
                     ygg::TreeFlags::WBT_GAMMA_NUMERATOR<2>,
                     ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<1>>;
using WBTSinglepass32TreeOptions =
    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE, ygg::TreeFlags::WBT_SINGLE_PASS,
                     ygg::TreeFlags::WBT_DELTA_NUMERATOR<3>,
                     ygg::TreeFlags::WBT_DELTA_DENOMINATOR<1>,
                     ygg::TreeFlags::WBT_GAMMA_NUMERATOR<2>,
                     ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<1>>;

using WBTSinglepassLWTreeOptions =
    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE, ygg::TreeFlags::WBT_SINGLE_PASS,
                     ygg::TreeFlags::WBT_DELTA_NUMERATOR<3>,
                     ygg::TreeFlags::WBT_DELTA_DENOMINATOR<1>,
                     ygg::TreeFlags::WBT_GAMMA_NUMERATOR<4>,
                     ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<3>>;

using WBTSinglepassBalTreeOptions =
    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE, ygg::TreeFlags::WBT_SINGLE_PASS,
                     ygg::TreeFlags::WBT_DELTA_NUMERATOR<2>,
                     ygg::TreeFlags::WBT_DELTA_DENOMINATOR<1>,
                     ygg::TreeFlags::WBT_GAMMA_NUMERATOR<3>,
                     ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<2>>;
using WBTSinglepassBalArithTreeOptions =
    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE, ygg::TreeFlags::WBT_SINGLE_PASS,
                     ygg::TreeFlags::WBT_DELTA_NUMERATOR<2>,
                     ygg::TreeFlags::WBT_DELTA_DENOMINATOR<1>,
                     ygg::TreeFlags::WBT_GAMMA_NUMERATOR<3>,
                     ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<2>,
                     ygg::TreeFlags::MICRO_AVOID_CONDITIONALS>;

#endif
