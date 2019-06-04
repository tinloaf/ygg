#ifndef TEST_MULTI_RBTREE_HPP
#define TEST_MULTI_RBTREE_HPP

#include <algorithm>
#include <gtest/gtest.h>
#include <random>
#include <vector>

#include "../src/rbtree.hpp"
#include "randomizer.hpp"

namespace ygg {
namespace testing {
namespace rbtree_multi {

using namespace ygg;

constexpr int TESTSIZE = 1000;

class TAG_A {
};
class TAG_B {
};

class Node
    : public RBTreeNodeBase<Node, TreeOptions<TreeFlags::MULTIPLE>, TAG_A>,
      public RBTreeNodeBase<Node, TreeOptions<TreeFlags::MULTIPLE>,
                            TAG_B> { // No multi-nodes!
public:
	int dataA;
	int dataB;

	Node() : dataA(0), dataB(0){};

	explicit Node(int dataA_in, int dataB_in)
	    : dataA(dataA_in), dataB(dataB_in){};
};

using NodeBaseA = RBTreeNodeBase<Node, TreeOptions<TreeFlags::MULTIPLE>, TAG_A>;
using NodeBaseB = RBTreeNodeBase<Node, TreeOptions<TreeFlags::MULTIPLE>, TAG_B>;

class CompareA {
public:
	bool
	operator()(const Node & lhs, const Node & rhs) const
	{
		return lhs.dataA < rhs.dataA;
	}
};

class CompareB {
public:
	bool
	operator()(const Node & lhs, const Node & rhs) const
	{
		return lhs.dataB < rhs.dataB;
	}
};

class NodeTraits {
public:
	static std::string
	get_id(const Node * node)
	{
		return std::to_string(node->dataA) + std::string(" / ") +
		       std::to_string(node->dataB);
	}

	template <class Tree>
	static void
	leaf_inserted(Node & node, Tree & t)
	{
		(void)node;
		(void)t;
	}

	template <class Tree>
	static void
	rotated_left(Node & node, Tree & t)
	{
		(void)node;
		(void)t;
	}

	template <class Tree>
	static void
	rotated_right(Node & node, Tree & t)
	{
		(void)node;
		(void)t;
	}

	template <class Tree>
	static void
	delete_leaf(Node & node, Tree & t)
	{
		(void)node;
		(void)t;
	}

	template <class Tree>
	static void
	swapped(Node & n1, Node & n2, Tree & t)
	{
		(void)n1;
		(void)n2;
		(void)t;
	}
};

class TreeA : public RBTree<Node, NodeTraits, TreeOptions<TreeFlags::MULTIPLE>,
                            TAG_A, CompareA> {
	using RBTree::RBTree;
};

class TreeB : public RBTree<Node, NodeTraits, TreeOptions<TreeFlags::MULTIPLE>,
                            TAG_B, CompareB> {
	using RBTree::RBTree;
};

TEST(MultiRBTreeTest, TrivialInsertionTest)
{
	auto ta = TreeA();
	auto tb = TreeB();

	Node n(0, 0);
	ta.insert(n);
	tb.insert(n);

	ASSERT_TRUE(ta.verify_integrity());
	ASSERT_TRUE(tb.verify_integrity());
}

TEST(MultiRBTreeTest, MinimalInteractionTest)
{
	auto ta = TreeA();
	auto tb = TreeB();

	class TestNode : public Node {
	public:
		using Node::Node;
	};

	TestNode n1(0, 0);
	TestNode n2(-1, 1);

	ta.insert(n1);
	ta.insert(n2);

	ASSERT_EQ(n1.NodeBaseA::get_right(), nullptr);
	ASSERT_EQ(n1.NodeBaseA::get_left(), &n2);
	ASSERT_EQ(n1.NodeBaseA::get_parent(), nullptr);
	ASSERT_EQ(n2.NodeBaseA::get_parent(), &n1);

	tb.insert(n1);
	tb.insert(n2);

	ASSERT_EQ(n1.NodeBaseA::get_right(), nullptr);
	ASSERT_EQ(n1.NodeBaseA::get_left(), &n2);
	ASSERT_EQ(n1.NodeBaseA::get_parent(), nullptr);
	ASSERT_EQ(n2.NodeBaseA::get_parent(), &n1);

	ASSERT_EQ(n1.NodeBaseB::get_right(), &n2);
	ASSERT_EQ(n1.NodeBaseB::get_left(), nullptr);
	ASSERT_EQ(n1.NodeBaseB::get_parent(), nullptr);
	ASSERT_EQ(n2.NodeBaseB::get_parent(), &n1);

	ASSERT_TRUE(ta.verify_integrity());
	ASSERT_TRUE(tb.verify_integrity());
}

TEST(MultiRBTreeTest, RandomInsertionTest)
{
	auto ta = TreeA();
	auto tb = TreeB();

	std::mt19937 rng(4); // chosen by fair xkcd
	std::uniform_int_distribution<int> uni(std::numeric_limits<int>::min(),
	                                       std::numeric_limits<int>::max());

	Node nodes[TESTSIZE];

	for (unsigned int i = 0; i < TESTSIZE; ++i) {
		int val_a = uni(rng);
		int val_b = uni(rng);

		nodes[i] = Node(val_a, val_b);

		ta.insert(nodes[i]);
		tb.insert(nodes[i]);

		// std::string fname = std::string("/tmp/trees/tree-") + std::to_string(i)
		// + std::string(".dot"); tree.dump_to_dot(fname);

		ASSERT_TRUE(ta.verify_integrity());
		ASSERT_TRUE(tb.verify_integrity());
	}

	int last_val = std::numeric_limits<int>::min();
	for (const auto & n : ta) {
		ASSERT_GE(n.dataA, last_val);
		last_val = n.dataA;
	}

	last_val = std::numeric_limits<int>::min();
	for (const auto & n : tb) {
		ASSERT_GE(n.dataB, last_val);
		last_val = n.dataB;
	}
}

TEST(MultiRBTreeTest, LinearInsertionTest)
{
	auto ta = TreeA();
	auto tb = TreeB();

	Node nodes[TESTSIZE];

	for (unsigned int i = 0; i < TESTSIZE; ++i) {
		nodes[i] = Node((int)i, (int)i);

		ta.insert(nodes[i]);
		tb.insert(nodes[i]);

		// std::string fname = std::string("/tmp/trees/tree-") + std::to_string(i)
		// + std::string(".dot"); tree.dump_to_dot(fname);

		ASSERT_TRUE(ta.verify_integrity());
		ASSERT_TRUE(ta.verify_integrity());
	}
}

TEST(MultiRBTreeTest, LinearIterationTest)
{
	auto ta = TreeA();
	auto tb = TreeB();

	Node nodes[TESTSIZE];

	std::vector<size_t> indices;
	for (unsigned int i = 0; i < TESTSIZE; ++i) {
		nodes[i] = Node((int)i, (int)(TESTSIZE - i));
		indices.push_back(i);
	}

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(4));

	for (auto index : indices) {
		ta.insert(nodes[index]);
		tb.insert(nodes[index]);
	}

	ASSERT_TRUE(ta.verify_integrity());
	ASSERT_TRUE(tb.verify_integrity());

	unsigned int i = 0;
	for (auto & n : ta) {
		ASSERT_EQ(n.dataA, i);
		i++;
	}

	i = 1;
	for (auto & n : tb) {
		ASSERT_EQ(n.dataB, i);
		i++;
	}
}

} // namespace rbtree_multi
} // namespace testing
} // namespace ygg

#endif // TEST_MULTI_RBTREE
