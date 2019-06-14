#ifndef BENCH_COMMON_BST_HPP
#define BENCH_COMMON_BST_HPP

#include "../src/ygg.hpp"
#include "benchmark.h"
#include "common.hpp"

#include <algorithm>
#include <boost/intrusive/set.hpp>
#include <draup.hpp>
#include <random>
#include <unordered_set>
#include <vector>

#ifdef USEPAPI
#include <papi.h>
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
struct WBBSTNamerBalSPArithFull
{
	constexpr static const char * name = "2,3/2,SP(opt),arith-ull";
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
struct RBBSTNamerArithFull
{
	constexpr static const char * name = "RBTree[arith-full] ";
};

// TODO various RBTree / Zip Tree variants!

template <class Interface, typename Experiment, bool need_nodes,
          bool need_values, bool need_node_pointers, bool values_from_fixed,
          bool distinct = false>
class BSTFixture : public benchmark::Fixture {
public:
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

	BSTFixture() : rng(std::random_device{}()) {}

	void
	SetUp(const ::benchmark::State & state)
	{
		Interface::clear(this->t);
		this->papi.initialize();

		size_t fixed_count = static_cast<size_t>(state.range(0));
		size_t experiment_count = static_cast<size_t>(state.range(1));
		int seed = static_cast<int>(state.range(2));
		this->rng = std::mt19937(static_cast<unsigned long>(seed));

		std::uniform_int_distribution<> distr(std::numeric_limits<int>::min(),
		                                      std::numeric_limits<int>::max());

		this->fixed_nodes.clear();
		std::unordered_set<int> seen_values;
		for (size_t i = 0; i < fixed_count; ++i) {
			int val = distr(this->rng);
			if (distinct) {
				while (seen_values.find(val) != seen_values.end()) {
					val = distr(this->rng);
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

		if (need_nodes) {
			seen_values.clear();
			this->experiment_nodes.clear();
			for (size_t i = 0; i < experiment_count; ++i) {
				int val;

				if (values_from_fixed) {
					val = fixed_values[i % fixed_count];
				} else {
					val = distr(this->rng);
					if (distinct) {
						while (seen_values.find(val) != seen_values.end()) {
							val = distr(this->rng);
						}
						seen_values.insert(val);
					}
				}

				this->experiment_nodes.push_back(Interface::create_node(val));
			}
		}

		if (need_values) {
			seen_values.clear();
			this->experiment_values.clear();
			for (size_t i = 0; i < experiment_count; ++i) {
				int val;

				if (values_from_fixed) {
					val = shuffled_values[i % fixed_count];
				} else {
					val = distr(this->rng);
					if (distinct) {
						while (seen_values.find(val) != seen_values.end()) {
							val = distr(this->rng);
						}
						seen_values.insert(val);
					}
				}

				this->experiment_values.push_back(val);
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
			    ptrs.begin() + static_cast<long>(experiment_count));
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

	std::mt19937 rng;

	typename Interface::Tree t;

	PapiMeasurements papi;
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
		int value;

	public:
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
using FullArithTreeOptions =
    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE, ygg::TreeFlags::ZTREE_USE_HASH,
                     ygg::TreeFlags::ZTREE_RANK_TYPE<uint8_t>,
                     ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT<
                         9859957398433823229ul>,
                     ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_MODUL<
                         std::numeric_limits<size_t>::max()>,
                     ygg::TreeFlags::MICRO_AVOID_CONDITIONALS,
                     ygg::TreeFlags::MICRO_AVOID_CONDITIONALS_SETTING,
                     ygg::TreeFlags::MICRO_DUMMY_SETTING_POINTER>;

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
using WBTSinglepassBalArithFullTreeOptions =
    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE, ygg::TreeFlags::WBT_SINGLE_PASS,
                     ygg::TreeFlags::WBT_DELTA_NUMERATOR<2>,
                     ygg::TreeFlags::WBT_DELTA_DENOMINATOR<1>,
                     ygg::TreeFlags::WBT_GAMMA_NUMERATOR<3>,
                     ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<2>,
                     ygg::TreeFlags::MICRO_AVOID_CONDITIONALS,
                     ygg::TreeFlags::MICRO_AVOID_CONDITIONALS_SETTING,
                     ygg::TreeFlags::MICRO_DUMMY_SETTING_POINTER>;

#endif
