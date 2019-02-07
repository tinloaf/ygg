#ifndef BENCH_COMMON_BST_HPP
#define BENCH_COMMON_BST_HPP

#include "benchmark.h"
#include <algorithm>
#include <boost/intrusive/set.hpp>
#include <random>
#include <vector>

#include "../src/ygg.hpp"

#include "main.hpp"

#ifdef USEPAPI
#include <papi.h>
#endif

// TODO various RBTree / Zip Tree variants!

template <class Interface, bool need_nodes, bool need_values,
          bool need_node_pointers, bool values_from_fixed>
class BSTFixture : public benchmark::Fixture {
public:
	BSTFixture() : rng(std::random_device{}())
	{
#ifdef USEPAPI
		this->papi_initialized = false;
#endif
	}

	void
	papi_initialize()
	{
#ifdef USEPAPI
		if (!this->papi_initialized) {
			this->papi_initialized = true;

			int num_counters = PAPI_num_counters();
			if (!PAPI_STATS_WRITTEN) {
				std::cout << "## Your system has " << num_counters
				          << " PAPI counters.\n";
			}

			if (num_counters == 0) {
				std::cout
				    << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
				std::cout
				    << "!!        Warning: No PAPI counters available!       !!\n";
				std::cout
				    << "!! Use the papi_avail and papi_component_avail tools !!\n";
				std::cout
				    << "!! to figure out why. If you are on Linux, you most  !!\n";
				std::cout
				    << "!! probably need to set                              !!\n";
				std::cout
				    << "!! /proc/sys/kernel/perf_event_paranoid to 0         !!\n";
				std::cout
				    << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
			}

			int event_code;
			event_code = 0 | PAPI_NATIVE_MASK;

			this->papi_selected_events.clear();
			for (const std::string & event_str : PAPI_MEASUREMENTS) {
				if (PAPI_event_name_to_code(event_str.c_str(), &event_code) !=
				    PAPI_OK) {
					std::cerr << "PAPI event " << event_str << " not found!\n";
					exit(-1);
				}

				this->papi_selected_events.push_back(event_code);
				if (!PAPI_STATS_WRITTEN) {
					std::cout << "## Registering PAPI event " << event_str << " (ID "
					          << event_code << ")\n";
				}
			}
			PAPI_STATS_WRITTEN = true;
		}

		this->papi_event_counts.resize(this->papi_selected_events.size());
		this->papi_event_count_accu.resize(this->papi_selected_events.size());
		std::fill(this->papi_event_count_accu.begin(),
		          this->papi_event_count_accu.end(), 0);
#endif
	}

	void
	papi_start()
	{
#ifdef USEPAPI
		// TODO error handling
		PAPI_start_counters(this->papi_selected_events.data(),
		                    (int)this->papi_selected_events.size());
#endif
	}

	void
	papi_stop()
	{
#ifdef USEPAPI
		// TODO error handling
		PAPI_stop_counters(this->papi_event_counts.data(),
		                   (int)this->papi_event_counts.size());
		std::transform(this->papi_event_counts.begin(),
		               this->papi_event_counts.end(),
		               this->papi_event_count_accu.begin(),
		               this->papi_event_count_accu.begin(), std::plus<long long>());
#endif
	}

	void
	papi_report_and_reset(::benchmark::State & state)
	{
#ifdef USEPAPI
		for (size_t i = 0; i < PAPI_MEASUREMENTS.size(); ++i) {
			state.counters[PAPI_MEASUREMENTS[i]] = this->papi_event_count_accu[i];
		}
		std::fill(this->papi_event_count_accu.begin(),
		          this->papi_event_count_accu.end(), 0);
#endif
	}

	void
	SetUp(const ::benchmark::State & state)
	{
		this->papi_initialize();

		size_t fixed_count = state.range(0);
		size_t experiment_count = state.range(1);

		std::uniform_int_distribution<> distr(std::numeric_limits<int>::min(),
		                                      std::numeric_limits<int>::max());

		this->fixed_nodes.clear();
		for (size_t i = 0; i < fixed_count; ++i) {
			int val = distr(this->rng);
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
			this->experiment_nodes.clear();
			for (size_t i = 0; i < experiment_count; ++i) {
				int val;

				if (values_from_fixed) {
					val = fixed_values[i % fixed_count];
				} else {
					val = distr(this->rng);
				}

				this->experiment_nodes.push_back(Interface::create_node(val));
			}
		}

		if (need_values) {
			for (size_t i = 0; i < experiment_count; ++i) {
				int val;

				if (values_from_fixed) {
					val = shuffled_values[i % fixed_count];
				} else {
					val = distr(this->rng);
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
			    ptrs.begin() + experiment_count);
		}
	}

	void
	TearDown(const ::benchmark::State & state)
	{
		Interface::clear(this->t);
	}

	std::vector<int> fixed_values;
	std::vector<typename Interface::Node> fixed_nodes;

	std::vector<typename Interface::Node> experiment_nodes;
	std::vector<int> experiment_values;
	std::vector<typename Interface::Node *> experiment_node_pointers;

	std::mt19937 rng;

	typename Interface::Tree t;

#ifdef USEPAPI
	std::vector<int> papi_selected_events;
	std::vector<long long> papi_event_counts;
	std::vector<long long> papi_event_count_accu;
	bool papi_initialized;
#endif
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

template <class MyTreeOptions>
class YggRBTreeInterface {
public:
	using Node = RBNode<MyTreeOptions>;
	using Tree = ygg::RBTree<Node, ygg::RBDefaultNodeTraits, MyTreeOptions>;

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
		return n.get_value();
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
	using Node = decltype(std::set<int>().extract(0));
	using Tree = std::multiset<int>;

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
    ygg::TreeOptions<ygg::TreeFlags::ZTREE_USE_HASH,
                     ygg::TreeFlags::ZTREE_RANK_TYPE<uint8_t>,
                     ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT<
                         9859957398433823229ul>,
                     ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_MODUL<
                         std::numeric_limits<size_t>::max()>>;

#endif
