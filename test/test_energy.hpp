#ifndef TEST_ENERGY_HPP
#define TEST_ENERGY_HPP

#include <algorithm>
#include <gtest/gtest.h>
#include <random>
#include <vector>

#include "../src/energy.hpp"
#include "randomizer.hpp"

namespace ygg {
namespace testing {
namespace energy {

using namespace ygg;

constexpr int ETREE_TESTSIZE = 1000;

class Node : public EnergyTreeNodeBase<Node> {
public:
	int data;

	Node() : data(0){};
	explicit Node(int data_in) : data(data_in){};
	Node(const Node & other) : data(other.data){};

	bool
	operator<(const Node & other) const
	{
		return this->data < other.data;
	}

	Node &
	operator=(const Node & other)
	{
		this->data = other.data;
		return *this;
	}
};

TEST(EnergyTreeTest, TrivialInsertionTest)
{
	auto tree =
		EnergyTree<Node>();

	Node n;
	n.data = 0;
	tree.insert(n);
	ASSERT_FALSE(tree.empty());
	ASSERT_TRUE(tree.verify_integrity());
}

TEST(EnergyTreeTest, RandomInsertionTest)
{
	auto tree =
		EnergyTree<Node>();

	std::mt19937 rng(4); // chosen by fair xkcd
	std::uniform_int_distribution<int> uni(std::numeric_limits<int>::min(),
	                                       std::numeric_limits<int>::max());

	Node nodes[ETREE_TESTSIZE];

	std::set<int> values_seen;
	for (unsigned int i = 0; i < ETREE_TESTSIZE; ++i) {
		int val = uni(rng);
		while (values_seen.find(val) != values_seen.end()) {
			val = uni(rng);
		}
		nodes[i] = Node(val);
		values_seen.insert(val);

		tree.insert(nodes[i]);

		ASSERT_TRUE(tree.verify_integrity());
	}
}

} // namespace energy
} // namespace testing
} // namespace ygg

#endif // TEST_ENERGY_HPP
