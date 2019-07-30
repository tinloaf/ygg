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

// TODO various RBTree / Zip Tree var-iants!

template <class Interface, typename Experiment, class MainRandomizer,
          class need_nodes, class need_values, class need_node_pointers,
          bool values_from_fixed, bool distinct = false,
          size_t node_value_change_percentage = 0>
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

		auto main_rnd = MainRandomizer::create(static_cast<unsigned long>(seed));

		this->fixed_nodes.clear();
		this->fixed_values.clear();

		std::unordered_set<int> seen_values;
		for (size_t i = 0; i < fixed_count; ++i) {
			int val = main_rnd.generate(MainRandomizer::min, MainRandomizer::max);
			if (distinct) {
				while (seen_values.find(val) != seen_values.end()) {
					val = main_rnd.generate(MainRandomizer::min, MainRandomizer::max);
				}
				seen_values.insert(val);
			}

			this->fixed_nodes.push_back(Interface::create_node(val));
			this->fixed_values.push_back(val);
		}
		for (auto & n : this->fixed_nodes) {
			Interface::insert(this->t, n);
		}

		std::vector<int> shuffled_values;
		if (values_from_fixed) {
			shuffled_values.insert(shuffled_values.begin(),
			                       this->fixed_values.begin(),
			                       this->fixed_values.end());
			std::shuffle(shuffled_values.begin(), shuffled_values.end(), this->rng);
		}

		if constexpr (need_nodes::enable) {
			seen_values.clear();
			auto rnd = need_nodes::create(rng());

			this->experiment_nodes.clear();
			for (size_t i = 0; i < experiment_count; ++i) {
				int val;

				if (values_from_fixed) {
					size_t rand_index = static_cast<size_t>(
					    rnd.generate(0, static_cast<int>(this->fixed_values.size())));
					val = fixed_values[rand_index];
				} else {
					val = rnd.generate(need_nodes::min, need_nodes::max);
					if (distinct) {
						while (seen_values.find(val) != seen_values.end()) {
							val = rnd.generate(need_nodes::min, need_nodes::max);
						}
						seen_values.insert(val);
					}
				}

				this->experiment_nodes.push_back(Interface::create_node(val));
			}
		}

		if constexpr (need_node_pointers::enable) {
			auto rnd = need_node_pointers::create(rng());
			std::unordered_set<typename Interface::Node *> seen_nodes;
			this->experiment_node_pointers.clear();
			for (size_t i = 0; i < experiment_count; ++i) {
				size_t rnd_index = static_cast<size_t>(
				    rnd.generate(0, static_cast<int>(this->fixed_nodes.size())));
				auto node_ptr = &this->fixed_nodes[rnd_index];
				if (distinct) {
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
		}

		if constexpr (need_values::enable) {
			seen_values.clear();
			auto rnd = need_values::create(rng());

			this->experiment_values.clear();
			for (size_t i = 0; i < experiment_count; ++i) {
				int val;

				if (values_from_fixed) {
					size_t rnd_index = static_cast<size_t>(
					    rnd.generate(0, static_cast<int>(this->fixed_values.size())));
					val = this->fixed_values[rnd_index];
				} else {
					int min = need_values::min;
					int max = need_values::max;

					if constexpr (node_value_change_percentage > 0) {
						double node_value_change =
						    static_cast<double>(node_value_change_percentage) / 100.0;
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
					if (distinct) {
						while (seen_values.find(val) != seen_values.end()) {
							val = rnd.generate(min, max);
						}
						seen_values.insert(val);
					}
				}

				this->experiment_values.push_back(val);
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
