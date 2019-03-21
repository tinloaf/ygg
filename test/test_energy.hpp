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

constexpr int ETREE_TESTSIZE = 5000;

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

// Make comparable to int
bool
operator<(const ygg::testing::energy::Node & lhs, const int rhs)
{
	return lhs.data < rhs;
}
bool
operator<(const int lhs, const ygg::testing::energy::Node & rhs)
{
	return lhs < rhs.data;
}

TEST(EnergyTreeTest, TrivialInsertionTest)
{
	auto tree = EnergyTree<Node>();

	Node n;
	n.data = 0;
	tree.insert(n);
	ASSERT_FALSE(tree.empty());
	ASSERT_TRUE(tree.verify_integrity());
}

TEST(EnergyTreeTest, TrivialFindTest)
{
	EnergyTree<Node> tree;

	Node n0(0);
	tree.insert(n0);
	ASSERT_TRUE(tree.verify_integrity());
	Node n1(1);
	tree.insert(n1);
	ASSERT_TRUE(tree.verify_integrity());

	Node n2(2);
	tree.insert(n2);
	ASSERT_TRUE(tree.verify_integrity());

	ASSERT_EQ(tree.find(n0), tree.begin());
	ASSERT_EQ(tree.find(0), tree.begin());
	ASSERT_TRUE(tree.find(2) != tree.end());
	ASSERT_TRUE(tree.find(n1) != tree.end());
	ASSERT_TRUE(tree.find(5) == tree.end());
}

TEST(EnergyTreeTest, RandomInsertionTest)
{
	auto tree = EnergyTree<Node>();

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

TEST(EnergyTreeTest, LinearInsertionTest)
{
	auto tree = EnergyTree<Node>();

	Node nodes[ETREE_TESTSIZE];

	for (unsigned int i = 0; i < ETREE_TESTSIZE; ++i) {
		nodes[i] = Node((int)i);

		tree.insert(nodes[i]);

		ASSERT_TRUE(tree.verify_integrity());
	}
}

TEST(EnergyTreeTest, TrivialDeletionTest)
{
	auto tree = EnergyTree<Node>();

	Node n1;
	n1.data = 0;
	tree.insert(n1);

	Node n2;
	n2.data = 1;
	tree.insert(n2);

	ASSERT_FALSE(tree.empty());
	ASSERT_TRUE(tree.verify_integrity());

	tree.remove(n2);

	ASSERT_TRUE(tree.verify_integrity());

	tree.remove(n1);

	ASSERT_TRUE(tree.verify_integrity());
	ASSERT_TRUE(tree.empty());
}

TEST(EnergyTreeTest, LinearInsertionLinearDeletionTest)
{
	auto tree = EnergyTree<Node>();

	Node nodes[ETREE_TESTSIZE];

	for (unsigned int i = 0; i < ETREE_TESTSIZE; ++i) {
		nodes[i] = Node((int)i);

		tree.insert(nodes[i]);
	}

	ASSERT_TRUE(tree.verify_integrity());

	for (unsigned int i = 0; i < ETREE_TESTSIZE; ++i) {
		// std::cout << "\n\n Removing " << i << "\n";
		tree.remove(nodes[i]);

		ASSERT_TRUE(tree.verify_integrity());
	}
}

TEST(EnergyTreeTest, LinearInsertionRandomDeletionTest)
{
	auto tree = EnergyTree<Node>();

	Node nodes[ETREE_TESTSIZE];
	std::vector<unsigned int> indices;

	for (unsigned int i = 0; i < ETREE_TESTSIZE; ++i) {
		nodes[i] = Node((int)i);

		tree.insert(nodes[i]);
		indices.push_back(i);
	}

	std::mt19937 rng(4); // chosen by fair xkcd
	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(4));

	ASSERT_TRUE(tree.verify_integrity());

	for (unsigned int i = 0; i < ETREE_TESTSIZE; ++i) {
		tree.remove(nodes[indices[i]]);

		ASSERT_TRUE(tree.verify_integrity());
	}
}

TEST(EnergyTreeTest, LinearIterationTest)
{
	auto tree = EnergyTree<Node>();

	Node nodes[ETREE_TESTSIZE * 5];

	std::vector<size_t> indices;

	for (unsigned int i = 0; i < ETREE_TESTSIZE; ++i) {
		for (unsigned j = 0; j < 5; ++j) {
			nodes[5 * i + j] = Node((int)i);
			indices.push_back(5 * i + j);
		}
	}

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(4));

	for (auto index : indices) {
		tree.insert(nodes[index]);
	}

	ASSERT_TRUE(tree.verify_integrity());

	unsigned int i = 0;
	for (auto & n : tree) {
		ASSERT_EQ(n.data, nodes[i].data);
		i++;
	}
}

TEST(EnergyTreeTest, ReverseIterationTest)
{
	auto tree = EnergyTree<Node>();

	Node nodes[ETREE_TESTSIZE];
	std::vector<size_t> indices;
	for (unsigned int i = 0; i < ETREE_TESTSIZE; ++i) {
		nodes[i] = Node((int)i);
		indices.push_back(i);
	}

	std::mt19937 rng(4); // chosen by fair xkcd
	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(4));

	for (auto index : indices) {
		tree.insert(nodes[index]);
	}

	ASSERT_TRUE(tree.verify_integrity());

	auto it = tree.rbegin();
	unsigned int i = ETREE_TESTSIZE - 1;
	while (it != tree.rend()) {
		ASSERT_EQ(it->data, i);
		it++;
		i--;
	}
}

TEST(EnergyTreeTest, ComprehensiveTest)
{
	auto tree = EnergyTree<Node>();

	Node persistent_nodes[ETREE_TESTSIZE];
	std::vector<unsigned int> indices;
	std::mt19937 rng(4); // chosen by fair xkcd

	std::set<unsigned int> values_seen;

	for (unsigned int i = 0; i < ETREE_TESTSIZE; ++i) {
		unsigned int data = 10 * i;
		persistent_nodes[i] = Node((int)data);
		indices.push_back(i);
		values_seen.insert(data);
	}

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(4));

	for (auto index : indices) {
		tree.insert(persistent_nodes[index]);
	}

	ASSERT_TRUE(tree.verify_integrity());

	Node transient_nodes[ETREE_TESTSIZE];
	for (unsigned int i = 0; i < ETREE_TESTSIZE; ++i) {
		std::uniform_int_distribution<unsigned int> uni(0,
		                                                10 * (ETREE_TESTSIZE + 1));
		unsigned int data = uni(rng);

		while (values_seen.find(data) != values_seen.end()) {
			data = uni(rng);
		}

		transient_nodes[i] = Node((int)data);

		values_seen.insert(data);
	}

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(4));

	for (auto index : indices) {
		tree.insert(transient_nodes[index]);
	}

	ASSERT_TRUE(tree.verify_integrity());

	for (int i = 0; i < ETREE_TESTSIZE; ++i) {
		tree.remove(transient_nodes[i]);

		ASSERT_TRUE(tree.verify_integrity());
	}

	// Query elements
	for (int i = 0; i < ETREE_TESTSIZE; ++i) {
		auto it = tree.find(persistent_nodes[i]);
		assert(&(*it) == &(persistent_nodes[i]));
		ASSERT_EQ(&(*it), &(persistent_nodes[i]));
	}
}

} // namespace energy
} // namespace testing
} // namespace ygg

#endif // TEST_ENERGY_HPP
