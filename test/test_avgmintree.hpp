#ifndef TEST_AVGMINTREE_HPP
#define TEST_AVGMINTREE_HPP

#include <algorithm>
#include <gtest/gtest.h>
#include <random>
#include <vector>

#include "../src/avgmintree.hpp"
#include "randomizer.hpp"

namespace ygg {
namespace testing {
namespace avgmintree {

using namespace ygg;

constexpr size_t AVGMINTREE_TESTSIZE = 5000;
constexpr size_t AVGMINTREE_SEED = 5;

using TestOptions = ygg::DefaultOptions;

class Node : public ygg::AvgMinTreeNodeBase<Node, TestOptions> {
public:
	int data;

	Node() : data(0){};
	Node(int data_in) : data(data_in){};
	Node(const Node & other) : data(other.data){};

	bool
	operator<(const Node & other) const
	{
		return this->data < other.data;
	}

	int
	get_data() const noexcept
	{
		return this->data;
	}
};

class NodeTraits : public AvgMinTreeDefaultNodeTraits<Node> {
public:
	static std::string get_id(Node * n) {
		return std::to_string(n->data);
	}
};
	
// Make comparable to int
bool
operator<(const ygg::testing::avgmintree::Node & lhs, const int rhs)
{
	return lhs.data < rhs;
}
bool
operator<(const int lhs, const ygg::testing::avgmintree::Node & rhs)
{
	return lhs < rhs.data;
}

using TestTree =
    AvgMinTree<Node, NodeTraits, TestOptions>;

TEST(AvgMinTreeTest, TrivialInsertionTest)
{
	TestTree tree;

	Node n(0);
	tree.insert(n);
	tree.dbg_verify();
}

TEST(AvgMinTreeTest, TrivialFindTest)
{
	TestTree tree;

	Node n0(0);
	tree.insert(n0);
	tree.dbg_verify();
	Node n1(1);
	tree.insert(n1);
	tree.dbg_print();
	tree.dbg_verify();
	Node n2(2);
	tree.insert(n2);
	tree.dbg_verify();

	ASSERT_EQ(tree.find(n0), tree.begin());
	ASSERT_EQ(tree.find(0), tree.begin());
	ASSERT_TRUE(tree.find(2) != tree.end());
	ASSERT_TRUE(tree.find(n1) != tree.end());
	ASSERT_TRUE(tree.find(5) == tree.end());
}

TEST(AvgMinTreeTest, TrivialUnzippingTest)
{
	TestTree tree;

	Node n0(0);
	Node n1(1);
	Node n2(2);
	Node n3(3);

	tree.insert(n0);
	tree.dbg_verify();

	tree.insert(n1);
	tree.dbg_verify();

	tree.insert(n2);
	tree.dbg_verify();

	tree.insert(n3);
	tree.dbg_verify();
}

TEST(AvgMinTreeTest, TrivialZippingTest)
{
	TestTree tree;

	Node n0(0);
	Node n1(1);
	Node n2(2);
	Node n3(3);

	tree.insert(n0);
	tree.insert(n1);
	tree.insert(n2);
	tree.insert(n3);

	tree.remove(n1);
	tree.dbg_print();
	tree.dbg_verify();

	tree.remove(n3);
	tree.dbg_verify();

	tree.remove(n2);
	tree.dbg_verify();

	tree.remove(n0);
	tree.dbg_verify();
}

TEST(AvgMinTreeTest, InsertionAndIterationTest)
{
	TestTree tree;

	Node nodes[AVGMINTREE_TESTSIZE];

	std::vector<size_t> indices;

	for (size_t i = 0; i < AVGMINTREE_TESTSIZE; ++i) {
		nodes[i] = Node((int)i);
		indices.push_back(i);
	}

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(AVGMINTREE_SEED));

	for (auto index : indices) {
		tree.insert(nodes[index]);
		//std::cout << "========================\n";
		//tree.dbg_print();
		tree.dbg_verify();
	}

	int i = 0;
	for (auto & node : tree) {
		ASSERT_EQ(node.get_data(), i);
		i++;
	}
}

TEST(AvgMinTreeTest, InsertionAndDeletionTest)
{
	TestTree tree;

	Node nodes[AVGMINTREE_TESTSIZE];

	std::vector<size_t> indices;
	std::vector<size_t> remove_indices;

	for (size_t i = 0; i < AVGMINTREE_TESTSIZE; ++i) {
		nodes[i] = Node((int)i);
		indices.push_back(i);
		remove_indices.push_back(i);
	}

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(AVGMINTREE_SEED));
	std::shuffle(remove_indices.begin(), remove_indices.end(),
	             ygg::testing::utilities::Randomizer(AVGMINTREE_SEED + 1));

	size_t i = 0;
	for (auto index : indices) {
		ASSERT_EQ(i, tree.size());
		tree.insert(nodes[index]);
		i++;
	}

	tree.dbg_verify();

	for (auto index : remove_indices) {
		tree.remove(nodes[index]);

		i--;
		ASSERT_EQ(i, tree.size());

		tree.dbg_verify();
	}

	ASSERT_TRUE(tree.empty());
	auto it = tree.begin();
	ASSERT_TRUE(it == tree.end());
}

TEST(AvgMinTreeTest, ComprehensiveTest)
{
	TestTree tree;

	Node persistent_nodes[AVGMINTREE_TESTSIZE];

	std::vector<unsigned int> indices;
	std::mt19937 rng(AVGMINTREE_SEED);

	std::set<unsigned int> values_seen;

	for (unsigned int i = 0; i < AVGMINTREE_TESTSIZE; ++i) {
		unsigned int data = 10 * i;
		persistent_nodes[i] = Node((int)data);
		indices.push_back(i);
		values_seen.insert(data);
	}

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(AVGMINTREE_SEED));

	for (auto index : indices) {
		tree.insert(persistent_nodes[index]);
	}

	tree.dbg_verify();

	Node transient_nodes[AVGMINTREE_TESTSIZE];

	for (unsigned int i = 0; i < AVGMINTREE_TESTSIZE; ++i) {
		std::uniform_int_distribution<unsigned int> uni(
		    0, 10 * (AVGMINTREE_TESTSIZE + 1));
		unsigned int data = uni(rng);

		while (values_seen.find(data) != values_seen.end()) {
			data = uni(rng);
		}

		transient_nodes[i] = Node((int)data);
		values_seen.insert(data);
	}

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(AVGMINTREE_SEED));

	for (auto index : indices) {
		tree.insert(transient_nodes[index]);
	}

	tree.dbg_verify();

	for (int i = 0; i < (int)AVGMINTREE_TESTSIZE; ++i) {
		tree.remove(transient_nodes[i]);
		//std::cout << "====================\n";
		//		tree.dbg_print();
		tree.dbg_verify();
	}

	// Query elements
	for (int i = 0; i < (int)AVGMINTREE_TESTSIZE; ++i) {
		auto it = tree.find(persistent_nodes[i]);
		assert(&(*it) == &(persistent_nodes[i]));
		ASSERT_EQ(&(*it), &(persistent_nodes[i]));
	}
}

} // namespace avgmintree
} // namespace testing
} // namespace ygg

#endif // TEST_AVGMINTREE_HPP
