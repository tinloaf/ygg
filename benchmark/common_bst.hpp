#ifndef BENCH_COMMON_HPP
#define BENCH_COMMON_HPP

#include "benchmark.h"
#include <algorithm>
#include <boost/intrusive/set.hpp>
#include <random>
#include <vector>

#include "../src/ygg.hpp"

// TODO various RBTree / Zip Tree variants!

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

		std::uniform_int_distribution<> distr(std::numeric_limits<int>::min(),
		                                      std::numeric_limits<int>::max());
		std::vector<int> fixed_values;

		this->fixed_nodes.clear();
		for (size_t i = 0; i < fixed_count; ++i) {
			int val = distr(this->rng);
			this->fixed_nodes.push_back(Interface::create_node(val));

			if (values_from_fixed) {
				fixed_values.push_back(val);
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
					val = fixed_values[i % fixed_count];
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

	std::vector<typename Interface::Node> fixed_nodes;

	std::vector<typename Interface::Node> experiment_nodes;
	std::vector<int> experiment_values;
	std::vector<typename Interface::Node *> experiment_node_pointers;

	std::mt19937 rng;

	typename Interface::Tree t;
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

template<class T>
bool operator<(const RBNode<T> & lhs, int rhs) {
	return lhs.get_value() < rhs;
}
template<class T>
bool operator<(int lhs, const RBNode<T> & rhs) {
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
template<class T>
bool operator<(const ZipNode<T> & lhs, int rhs) {
	return lhs.get_value() < rhs;
}
template<class T>
bool operator<(int lhs, const ZipNode<T> & rhs) {
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
	          boost ::intrusive::link_mode_type::normal_link>> {
		int value;

	public:
		Node(int value_in) : value(value_in) {}

		bool
		operator<(const Node & rhs) const
		{
			return this->value < rhs.value;
		}
	};

	using Tree = boost::intrusive::set<Node>;

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

using BasicTreeOptions =
    ygg::TreeOptions<ygg::TreeFlags::ZTREE_USE_HASH,
                     ygg::TreeFlags::ZTREE_RANK_TYPE<uint8_t>,
                     ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT<
                         9859957398433823229ul>,
                     ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_MODUL<
                         std::numeric_limits<size_t>::max()>>;

#endif
