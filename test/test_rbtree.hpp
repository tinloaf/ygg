#ifndef TEST_RBTREE_HPP
#define TEST_RBTREE_HPP

#include <algorithm>
#include <gtest/gtest.h>
#include <random>
#include <vector>

#include "../src/rbtree.hpp"
#include "randomizer.hpp"

namespace ygg {
namespace testing {
namespace rbtree {

using namespace ygg;

constexpr int RBTREE_TESTSIZE = 2000;

class Node
    : public RBTreeNodeBase<
          Node, TreeOptions<TreeFlags::COMPRESS_COLOR>> { // No multi-nodes!
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

class EqualityNode : public RBTreeNodeBase<EqualityNode> {
public:
	int data;
	int sub_data;

	EqualityNode() : data(0){};
	explicit EqualityNode(int data_in, int sub_data_in = 0)
	    : data(data_in), sub_data(sub_data_in){};
	EqualityNode(const EqualityNode & other)
	    : data(other.data), sub_data(other.sub_data){};

	bool
	operator<(const EqualityNode & other) const
	{
		return this->data < other.data;
	}

	EqualityNode &
	operator=(const EqualityNode & other)
	{
		this->data = other.data;
		this->sub_data = other.sub_data;
		return *this;
	}
};

class NodeTraits : public RBDefaultNodeTraits {
public:
	static std::string
	get_id(const Node * node)
	{
		return std::to_string(node->data);
	}
};

class EqualityNodeTraits : public RBDefaultNodeTraits {
public:
	static std::string
	get_id(const EqualityNode * node)
	{
		return std::string("(") + std::to_string(node->data) + std::string("/") +
		       std::to_string(node->sub_data) + std::string(")");
	}
};

TEST(RBTreeTest, TrivialInsertionTest)
{
	auto tree =
	    RBTree<Node, NodeTraits, TreeOptions<TreeFlags::COMPRESS_COLOR>>();

	Node n;
	n.data = 0;
	tree.insert(n);
	ASSERT_FALSE(tree.empty());
	ASSERT_TRUE(tree.verify_integrity());
}

TEST(RBTreeTest, TrivialSizeTest)
{
	auto tree = RBTree<EqualityNode, EqualityNodeTraits>();

	EqualityNode n;
	n.data = 0;

	ASSERT_EQ(tree.size(), 0);
	ASSERT_TRUE(tree.empty());

	tree.insert(n);

	ASSERT_EQ(tree.size(), 1);
	ASSERT_FALSE(tree.empty());

	tree.clear();
	ASSERT_EQ(tree.size(), 0);
	ASSERT_TRUE(tree.empty());
}

TEST(RBTreeTest, RandomInsertionTest)
{
	auto tree =
	    RBTree<Node, NodeTraits, TreeOptions<TreeFlags::COMPRESS_COLOR>>();

	std::mt19937 rng(4); // chosen by fair xkcd
	std::uniform_int_distribution<int> uni(std::numeric_limits<int>::min(),
	                                       std::numeric_limits<int>::max());

	Node nodes[RBTREE_TESTSIZE];

	std::set<int> values_seen;
	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
		int val = uni(rng);
		while (values_seen.find(val) != values_seen.end()) {
			val = uni(rng);
		}
		nodes[i] = Node(val);
		values_seen.insert(val);

		tree.insert(nodes[i]);

		// std::string fname = std::string("/tmp/trees/tree-") + std::to_string(i) +
		// std::string(".dot"); tree.dump_to_dot(fname);

		ASSERT_TRUE(tree.verify_integrity());
	}
}

TEST(RBTreeTest, CopyAssignmentTest)
{
	auto src = RBTree<EqualityNode, EqualityNodeTraits>();
	auto dst = RBTree<EqualityNode, EqualityNodeTraits>();

	std::mt19937 rng(4); // chosen by fair xkcd
	std::uniform_int_distribution<int> uni(std::numeric_limits<int>::min(),
	                                       std::numeric_limits<int>::max());

	EqualityNode nodes_src[RBTREE_TESTSIZE];
	EqualityNode nodes_dst[RBTREE_TESTSIZE];

	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
		int val = uni(rng);

		nodes_src[i] = EqualityNode(val, (int)i);
		nodes_dst[i] = EqualityNode(0, (int)i);

		src.insert(nodes_src[i]);
		dst.insert(nodes_dst[i]);
	}

	ASSERT_TRUE(src.verify_integrity());
	ASSERT_TRUE(dst.verify_integrity());

	dst.mimic(src);

	ASSERT_TRUE(src.verify_integrity());
	ASSERT_TRUE(dst.verify_integrity());

	auto src_it = src.begin();
	auto dst_it = dst.begin();

	while (src_it != src.end()) {
		ASSERT_TRUE(dst_it != dst.end());

		ASSERT_EQ(src_it->data, dst_it->data);
		ASSERT_EQ(src_it->sub_data, dst_it->sub_data);

		++src_it;
		++dst_it;
	}
}

TEST(RBTreeTest, LinearInsertionTest)
{
	auto tree =
	    RBTree<Node, NodeTraits, TreeOptions<TreeFlags::COMPRESS_COLOR>>();

	Node nodes[RBTREE_TESTSIZE];

	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
		nodes[i] = Node((int)i);

		tree.insert(nodes[i]);

		ASSERT_TRUE(tree.verify_integrity());
	}
}

TEST(RBTreeTest, HintedPostEqualInsertionTest)
{
	auto tree = RBTree<EqualityNode, EqualityNodeTraits>();

	EqualityNode n_insert_before(1, 0);
	EqualityNode n_pre(1, 1);
	EqualityNode n_insert_between(1, 2);
	EqualityNode n_post(2, 3);

	tree.insert(n_pre);
	tree.insert(n_post);

	ASSERT_TRUE(tree.verify_integrity());

	// should be inserted before pre
	tree.insert(n_insert_before, n_pre);

	// should be inserted between pre and post
	tree.insert(n_insert_between, n_post);

	auto it = tree.begin();
	ASSERT_EQ(it->sub_data, 0);
	it++;
	ASSERT_EQ(it->sub_data, 1);
	it++;
	ASSERT_EQ(it->sub_data, 2);
	it++;
	ASSERT_EQ(it->sub_data, 3);
	it++;
	ASSERT_EQ(it, tree.end());
}

TEST(RBTreeTest, RepeatedHintedPostEqualInsertionTest)
{
	auto tree = RBTree<EqualityNode, EqualityNodeTraits>();

	EqualityNode nodes_pre[RBTREE_TESTSIZE];
	EqualityNode nodes_post[RBTREE_TESTSIZE];
	EqualityNode nodes_between[RBTREE_TESTSIZE];
	EqualityNode node_border_small(1, RBTREE_TESTSIZE + 2);
	EqualityNode node_border_large(2, RBTREE_TESTSIZE + 2);

	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
		nodes_pre[i] = EqualityNode(1, (int)i);
		nodes_post[i] = EqualityNode(2, (int)i);
		nodes_between[i] = EqualityNode(1, (int)RBTREE_TESTSIZE + 1);
	}

	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
		tree.insert(nodes_post[i], tree.end()); // insert in order
	}

	tree.insert(nodes_pre[RBTREE_TESTSIZE - 1], nodes_post[0]);

	for (int i = RBTREE_TESTSIZE - 2; i >= 0; --i) {
		tree.insert(nodes_pre[i], nodes_pre[i + 1]);
		ASSERT_EQ(tree.begin()->sub_data, i);
	}

	for (int i = 0; i < RBTREE_TESTSIZE; ++i) {
		tree.insert(nodes_between[i], nodes_pre[i]);
	}

	tree.insert(node_border_large, nodes_post[0]);
	tree.insert(node_border_small, node_border_large);
	ASSERT_TRUE(tree.verify_integrity());

	auto it = tree.begin();
	for (int i = 0; i < RBTREE_TESTSIZE; ++i) {
		ASSERT_EQ(it->data, 1);
		ASSERT_EQ(it->sub_data, RBTREE_TESTSIZE + 1); // first, the 'between' node
		it++;
		ASSERT_EQ(it->data, 1);
		ASSERT_EQ(it->sub_data, i); // now, the pre-node
		it++;
	}

	ASSERT_EQ(it->data, 1);
	ASSERT_EQ(it->sub_data, RBTREE_TESTSIZE + 2); // small border
	it++;
	ASSERT_EQ(it->data, 2);
	ASSERT_EQ(it->sub_data, RBTREE_TESTSIZE + 2); // large border
	it++;

	for (int i = 0; i < RBTREE_TESTSIZE; ++i) {
		ASSERT_EQ(it->data, 2);
		ASSERT_EQ(it->sub_data, i); // post-nodes
		it++;
	}
}

TEST(RBTreeTest, LinearEndHintedInsertionTest)
{
	auto tree =
	    RBTree<Node, NodeTraits, TreeOptions<TreeFlags::COMPRESS_COLOR>>();

	Node nodes[RBTREE_TESTSIZE];

	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
		nodes[i] = Node((int)i);
	}

	tree.insert(nodes[RBTREE_TESTSIZE - 1]);

	for (int i = RBTREE_TESTSIZE - 2; i >= 0; --i) {
		tree.insert(nodes[i], nodes[RBTREE_TESTSIZE - 1]);
		ASSERT_TRUE(tree.verify_integrity());
	}

	int i = 0;
	for (const auto & n : tree) {
		ASSERT_EQ(n.data, i);
		i++;
	}
}

TEST(RBTreeTest, HinterOrderPreservationTest)
{
	auto tree = RBTree<EqualityNode, EqualityNodeTraits>();

	EqualityNode nodes[3 * RBTREE_TESTSIZE];

	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
		nodes[3 * i] = EqualityNode((int)i, 0);
		nodes[3 * i + 1] = EqualityNode((int)i, 1);
		nodes[3 * i + 2] = EqualityNode((int)i, 2);
	}

	// insert the middles
	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
		tree.insert(nodes[3 * i + 1]);
	}

	tree.verify_integrity();

	// insert the prefix, using a hint
	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
		tree.insert(nodes[3 * i], nodes[3 * i + 1]);
	}

	tree.verify_integrity();

	// insert the postfix, using a hint
	for (unsigned int i = 0; i < RBTREE_TESTSIZE - 1; ++i) {
		tree.insert(nodes[3 * i + 2], nodes[3 * i + 3]);
	}

	unsigned int i = 0;
	for (auto & n : tree) {
		ASSERT_EQ(n.data, i / 3);
		ASSERT_EQ(n.sub_data, i % 3);
		++i;
	}
}

TEST(RBTreeTest, LinearNextHintedInsertionTest)
{
	auto tree =
	    RBTree<Node, NodeTraits, TreeOptions<TreeFlags::COMPRESS_COLOR>>();

	Node nodes[RBTREE_TESTSIZE];

	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
		nodes[i] = Node((int)i);
	}

	tree.insert(nodes[RBTREE_TESTSIZE - 1]);

	for (int i = RBTREE_TESTSIZE - 2; i >= 0; --i) {
		tree.insert(nodes[i], nodes[i + 1]);
		ASSERT_TRUE(tree.verify_integrity());
	}

	int i = 0;
	for (const auto & n : tree) {
		ASSERT_EQ(n.data, i);
		i++;
	}
}

TEST(RBTreeTest, LowerBoundTest)
{
	auto tree =
	    RBTree<Node, NodeTraits, TreeOptions<TreeFlags::COMPRESS_COLOR>>();

	Node nodes[RBTREE_TESTSIZE];

	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
		nodes[i] = Node((int)(2 * i));
		tree.insert(nodes[i]);
	}

	ASSERT_TRUE(tree.verify_integrity());

	for (unsigned int i = 0; i < RBTREE_TESTSIZE - 1; ++i) {
		Node query_next((int)(2 * i + 1));
		auto it_next = tree.lower_bound(query_next);
		ASSERT_EQ(it_next->data, nodes[i + 1].data);

		Node query((int)(2 * i));
		auto it = tree.lower_bound(query);
		// We look for "not less"
		ASSERT_EQ(it->data, nodes[i].data);
	}

	Node query(2 * (RBTREE_TESTSIZE - 1) + 1);
	auto it = tree.lower_bound(query);
	ASSERT_EQ(it, tree.end());
}

TEST(RBTreeTest, UpperBoundTest)
{
	auto tree =
	    RBTree<Node, NodeTraits, TreeOptions<TreeFlags::COMPRESS_COLOR>>();

	Node nodes[RBTREE_TESTSIZE];

	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
		nodes[i] = Node((int)(2 * i));
		tree.insert(nodes[i]);
	}

	ASSERT_TRUE(tree.verify_integrity());

	for (unsigned int i = 0; i < RBTREE_TESTSIZE - 1; ++i) {
		Node query_next((int)(2 * i + 1));
		auto it_next = tree.upper_bound(query_next);
		ASSERT_EQ(it_next->data, nodes[i + 1].data);

		Node query((int)(2 * i));
		auto it = tree.upper_bound(query);
		// We look for "truly greater"
		ASSERT_EQ(it->data, nodes[i + 1].data);
	}

	Node query(2 * (RBTREE_TESTSIZE - 1) + 1);
	auto it = tree.upper_bound(query);
	ASSERT_EQ(it, tree.end());
}

TEST(RBTreeTest, TrivialDeletionTest)
{
	auto tree =
	    RBTree<Node, NodeTraits, TreeOptions<TreeFlags::COMPRESS_COLOR>>();

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

TEST(RBTreeTest, LinearInsertionLinearDeletionTest)
{
	auto tree =
	    RBTree<Node, NodeTraits, TreeOptions<TreeFlags::COMPRESS_COLOR>>();

	Node nodes[RBTREE_TESTSIZE];

	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
		nodes[i] = Node((int)i);

		tree.insert(nodes[i]);
	}

	ASSERT_TRUE(tree.verify_integrity());

	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
		// std::cout << "\n\n Removing " << i << "\n";
		tree.remove(nodes[i]);

		ASSERT_TRUE(tree.verify_integrity());
	}
}

TEST(RBTreeTest, LinearInsertionRandomDeletionTest)
{
	auto tree =
	    RBTree<Node, NodeTraits, TreeOptions<TreeFlags::COMPRESS_COLOR>>();

	Node nodes[RBTREE_TESTSIZE];
	std::vector<unsigned int> indices;

	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
		nodes[i] = Node((int)i);

		tree.insert(nodes[i]);
		indices.push_back(i);
	}

	std::mt19937 rng(4); // chosen by fair xkcd
	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(4));

	ASSERT_TRUE(tree.verify_integrity());

	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
		tree.remove(nodes[indices[i]]);
		ASSERT_TRUE(tree.verify_integrity());
	}
}

TEST(RBTreeTest, LinearMultipleIterationTest)
{
	auto tree = RBTree<EqualityNode, EqualityNodeTraits>();

	EqualityNode nodes[RBTREE_TESTSIZE * 5];

	std::vector<size_t> indices;

	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
		for (unsigned j = 0; j < 5; ++j) {
			nodes[5 * i + j] = EqualityNode((int)i);
			indices.push_back(5 * i + j);
		}
	}

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(4));

	size_t size = 0;
	for (auto index : indices) {
		tree.insert(nodes[index]);
		size++;
		ASSERT_EQ(tree.size(), size);
	}

	ASSERT_TRUE(tree.verify_integrity());

	unsigned int i = 0;
	for (auto & n : tree) {
		ASSERT_EQ(n.data, nodes[i].data);
		i++;
	}
}

TEST(RBTreeTest, LinearIterationTest)
{
	auto tree =
	    RBTree<Node, NodeTraits, TreeOptions<TreeFlags::COMPRESS_COLOR>>();

	Node nodes[RBTREE_TESTSIZE];
	std::vector<size_t> indices;
	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
		nodes[i] = Node((int)i);
		indices.push_back(i);
	}

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(4));

	for (auto index : indices) {
		tree.insert(nodes[index]);
	}

	ASSERT_TRUE(tree.verify_integrity());

	unsigned int i = 0;
	for (auto & n : tree) {
		ASSERT_EQ(n.data, i);
		i++;
	}
}

TEST(RBTreeTest, ReverseIterationTest)
{
	auto tree =
	    RBTree<Node, NodeTraits, TreeOptions<TreeFlags::COMPRESS_COLOR>>();

	Node nodes[RBTREE_TESTSIZE];
	std::vector<size_t> indices;
	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
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
	unsigned int i = RBTREE_TESTSIZE - 1;
	while (it != tree.rend()) {
		ASSERT_EQ(it->data, i);
		it++;
		i--;
	}
}

TEST(RBTreeTest, FindTest)
{
	auto tree =
	    RBTree<Node, NodeTraits, TreeOptions<TreeFlags::COMPRESS_COLOR>>();

	Node nodes[RBTREE_TESTSIZE];

	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
		nodes[i] = Node((int)(2 * i));
		tree.insert(nodes[i]);
	}

	// Nonexisting
	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
		Node findme((int)(2 * i + 1));
		auto it = tree.find(findme);
		ASSERT_EQ(it, tree.end());
	}

	// Existing
	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
		Node findme((int)(2 * i));
		auto it = tree.find(findme);
		ASSERT_EQ(&(*it), &(nodes[i]));
	}
}

TEST(RBTreeTest, ComprehensiveTest)
{
	auto tree =
	    RBTree<Node, NodeTraits, TreeOptions<TreeFlags::COMPRESS_COLOR>>();

	Node persistent_nodes[RBTREE_TESTSIZE];
	std::vector<unsigned int> indices;
	std::mt19937 rng(4); // chosen by fair xkcd

	std::set<unsigned int> values_seen;

	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
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

	Node transient_nodes[RBTREE_TESTSIZE];
	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
		std::uniform_int_distribution<unsigned int> uni(0,
		                                                10 * (RBTREE_TESTSIZE + 1));
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

	for (int i = 0; i < RBTREE_TESTSIZE; ++i) {
		tree.remove(transient_nodes[i]);

		ASSERT_TRUE(tree.verify_integrity());
	}

	// Query elements
	for (int i = 0; i < RBTREE_TESTSIZE; ++i) {
		auto it = tree.find(persistent_nodes[i]);
		assert(&(*it) == &(persistent_nodes[i]));
		ASSERT_EQ(&(*it), &(persistent_nodes[i]));
	}
}

TEST(RBTreeTest, ComprehensiveMultipleTest)
{
	auto tree = RBTree<EqualityNode, EqualityNodeTraits>();

	EqualityNode persistent_nodes[RBTREE_TESTSIZE];
	std::vector<unsigned int> indices;
	std::mt19937 rng(4); // chosen by fair xkcd

	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
		unsigned int data = 10 * i;
		persistent_nodes[i] = EqualityNode((int)data);
		indices.push_back(i);
	}

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(4));

	size_t size = 0;
	for (auto index : indices) {
		tree.insert(persistent_nodes[index]);
		size++;
	}

	ASSERT_TRUE(tree.verify_integrity());

	EqualityNode transient_nodes[RBTREE_TESTSIZE];
	for (unsigned int i = 0; i < RBTREE_TESTSIZE; ++i) {
		std::uniform_int_distribution<unsigned int> uni(0,
		                                                10 * (RBTREE_TESTSIZE + 1));
		unsigned int data = uni(rng);

		transient_nodes[i] = EqualityNode((int)data);
		// std::cout << "Inserting random value: " << data << "\n";
	}

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(4));

	for (auto index : indices) {
		tree.insert(transient_nodes[index]);
		size++;
	}

	ASSERT_EQ(tree.size(), size);
	ASSERT_TRUE(tree.verify_integrity());

	// std::string fname_before =
	// std::string("/tmp/trees/rbt-comprehensive-before.dot");
	// tree.dump_to_dot(fname_before);

	for (int i = 0; i < RBTREE_TESTSIZE; ++i) {
		tree.remove(transient_nodes[i]);
		size--;
		// std::string rem_fname = std::string("/tmp/trees/removed-") +
		// std::to_string(i) + std::string(".dot"); std::cout << "Step " << i << ":
		// removing data " << transient_nodes[i].data << "\n";
		// tree.dump_to_dot(rem_fname);
		ASSERT_TRUE(tree.verify_integrity());
		ASSERT_EQ(tree.size(), size);
	}

	// std::string fname = std::string("/tmp/trees/rbt-comprehensive.dot");
	// tree.dump_to_dot(fname);

	// Query elements
	for (int i = 0; i < RBTREE_TESTSIZE; ++i) {
		// std::cout << "Finding " << i << "\n";
		auto it = tree.find(persistent_nodes[i]);
		assert(&(*it) == &(persistent_nodes[i]));
		ASSERT_EQ(&(*it), &(persistent_nodes[i]));
	}
}
// TODO test equal elements

} // namespace rbtree
} // namespace testing
} // namespace ygg

#endif // TEST_RBTREE_HPP
