#ifndef TEST_ZIPTREE_HPP
#define TEST_ZIPTREE_HPP

#include "../src/ziptree.hpp"
#include "randomizer.hpp"

#include <algorithm>
#include <gtest/gtest.h>
#include <random>
#include <vector>

namespace ygg {
namespace testing {
namespace ziptree {

struct DummyOpt;
using namespace ygg;

constexpr size_t ZIPTREE_TESTSIZE = 5000;
constexpr size_t ZIPTREE_SEED = 4;

template <class AddOpt = DummyOpt>
using ExplicitRankOptions =
    ygg::TreeOptions<TreeFlags::MULTIPLE, TreeFlags::CONSTANT_TIME_SIZE,
                     TreeFlags::ZTREE_RANK_TYPE<int>, AddOpt>;
template <class AddOpt = DummyOpt>
using ImplicitRankOptions =
    ygg::TreeOptions<TreeFlags::MULTIPLE, TreeFlags::CONSTANT_TIME_SIZE,
                     TreeFlags::ZTREE_USE_HASH,
                     TreeFlags::ZTREE_RANK_TYPE<size_t>, AddOpt>;

template <class AddOpt = DummyOpt>
class NodeBase
    : public ZTreeNodeBase<NodeBase<AddOpt>, ExplicitRankOptions<AddOpt>> {
public:
	int data;
	int rank;

	NodeBase() : data(0), rank(0){};
	NodeBase(int data_in, int rank_in) : data(data_in), rank(rank_in){};

	bool
	operator<(const NodeBase<AddOpt> & other) const
	{
		return this->data < other.data;
	}

	int
	get_data() const noexcept
	{
		return this->data;
	}

	int
	get_rank() const noexcept
	{
		return this->rank;
	}
};

template <class AddOpt = DummyOpt>
class NodeTraits : public ZTreeDefaultNodeTraits<NodeBase<AddOpt>> {
public:
	static std::string
	get_id(const NodeBase<AddOpt> * node)
	{
		return std::to_string(node->get_data()) + std::string("@") +
		       std::to_string(node->get_rank());
	}
};

class DataRankGetter {
public:
	template <class AddOpt>
	static size_t
	get_rank(const NodeBase<AddOpt> & n)
	{
		return (size_t)n.get_rank();
	}
};

template <class AddOpt = DummyOpt>
class HashRankNodeBase : public ZTreeNodeBase<HashRankNodeBase<AddOpt>,
                                              ImplicitRankOptions<AddOpt>> {
public:
	int data;

	HashRankNodeBase() : data(0) { this->update_rank(); };
	HashRankNodeBase(int data_in) : data(data_in) { this->update_rank(); };
	HashRankNodeBase(const HashRankNodeBase<AddOpt> & other) : data(other.data)
	{
		this->update_rank();
	};

	bool
	operator<(const HashRankNodeBase<AddOpt> & other) const
	{
		return this->data < other.data;
	}

	int
	get_data() const noexcept
	{
		return this->data;
	}

	HashRankNodeBase<AddOpt> &
	operator=(const HashRankNodeBase<AddOpt> & other)
	{
		this->data = other.data;
		return *this;
	}

	void
	set_from(const HashRankNodeBase<AddOpt> & other)
	{
		*this = other;
		this->update_rank();
	}
};

template <class AddOpt = DummyOpt>
class HashRankNodeTraits
    : public ZTreeDefaultNodeTraits<HashRankNodeBase<AddOpt>> {
public:
	static std::string
	get_id(const HashRankNodeBase<AddOpt> * node)
	{
		return std::to_string(node->get_data());
	}
};

// Make comparable to int
template <class AddOpt>
bool
operator<(const ygg::testing::ziptree::NodeBase<AddOpt> & lhs, const int rhs)
{
	return lhs.data < rhs;
}

template <class AddOpt>
bool
operator<(const int lhs, const ygg::testing::ziptree::NodeBase<AddOpt> & rhs)
{
	return lhs < rhs.data;
}

template <class AddOpt>
bool
operator<(const ygg::testing::ziptree::HashRankNodeBase<AddOpt> & lhs,
          const int rhs)
{
	return lhs.data < rhs;
}

template <class AddOpt>
bool
operator<(const int lhs,
          const ygg::testing::ziptree::HashRankNodeBase<AddOpt> & rhs)
{
	return lhs < rhs.data;
}

} // namespace ziptree
} // namespace testing
} // namespace ygg

namespace std {
template <class AddOpt>
struct hash<ygg::testing::ziptree::HashRankNodeBase<AddOpt>>
{
	size_t
	operator()(const ygg::testing::ziptree::HashRankNodeBase<AddOpt> & n) const
	    noexcept
	{
		return hash<int>{}(n.get_data());
	}
};

} // namespace std

namespace ygg {
namespace testing {
namespace ziptree {

using Node = NodeBase<>;
using HashRankNode = HashRankNodeBase<>;

template <class AddOpt = DummyOpt>
using ExplicitRankTreeBase =
    ZTree<NodeBase<AddOpt>, NodeTraits<AddOpt>, ExplicitRankOptions<AddOpt>,
          int, ygg::utilities::flexible_less, DataRankGetter>;
using ExplicitRankTree = ExplicitRankTreeBase<>;

template <class AddOpt = DummyOpt>
using ImplicitRankTreeBase =
    ZTree<HashRankNodeBase<AddOpt>, HashRankNodeTraits<AddOpt>,
          ImplicitRankOptions<AddOpt>>;
using ImplicitRankTree = ImplicitRankTreeBase<>;

TEST(ZipTreeTest, TrivialInsertionTest)
{
	ExplicitRankTree tree;

	Node n(0, 0);
	tree.insert(n);
	tree.dbg_verify();

	ImplicitRankTree itree;

	HashRankNode in(0);
	itree.insert(in);
	itree.dbg_verify();
}

TEST(ZipTreeTest, TrivialFindTest)
{
	ExplicitRankTree tree;

	Node n0(0, 0);
	tree.insert(n0);
	tree.dbg_verify();
	Node n1(1, 5);
	tree.insert(n1);
	tree.dbg_verify();
	Node n2(2, 3);
	tree.insert(n2);
	tree.dbg_verify();

	ASSERT_EQ(tree.find(n0), tree.begin());
	ASSERT_EQ(tree.find(0), tree.begin());
	ASSERT_TRUE(tree.find(2) != tree.end());
	ASSERT_TRUE(tree.find(n1) != tree.end());
	ASSERT_TRUE(tree.find(5) == tree.end());

	ImplicitRankTree itree;

	HashRankNode in0(0);
	itree.insert(in0);
	HashRankNode in1(1);
	itree.insert(in1);
	HashRankNode in2(2);
	itree.insert(in2);
	itree.dbg_verify();

	ASSERT_EQ(itree.find(in0), itree.begin());
	ASSERT_EQ(itree.find(0), itree.begin());
	ASSERT_TRUE(itree.find(2) != itree.end());
	ASSERT_TRUE(itree.find(in1) != itree.end());
	ASSERT_TRUE(itree.find(5) == itree.end());
}

TEST(ZipTreeTest, TrivialUnzippingTest)
{
	ExplicitRankTree tree;
	ImplicitRankTree itree;

	Node n0(0, 1);
	Node n1(1, 2);
	Node n2(2, 0);
	Node n3(3, 1);

	HashRankNode in0(32);
	HashRankNode in1(36);
	HashRankNode in2(2);
	HashRankNode in3(3);

	tree.insert(n0);
	tree.dbg_verify();

	itree.insert(in0);
	itree.dbg_verify();

	tree.insert(n1);
	tree.dbg_verify();

	itree.insert(in1);
	itree.dbg_verify();

	tree.insert(n2);
	tree.dbg_verify();

	itree.insert(in2);
	itree.dbg_verify();

	tree.insert(n3);
	tree.dbg_verify();

	itree.insert(in3);
	itree.dbg_verify();
}

TEST(ZipTreeTest, TrivialZippingTest)
{
	ExplicitRankTree tree;
	ImplicitRankTree itree;

	Node n0(0, 1);
	Node n1(1, 2);
	Node n2(2, 0);
	Node n3(3, 1);

	HashRankNode in0(0);
	HashRankNode in1(1);
	HashRankNode in2(2);
	HashRankNode in3(3);

	tree.insert(n0);
	tree.insert(n1);
	tree.insert(n2);
	tree.insert(n3);
	itree.insert(in0);
	itree.insert(in1);
	itree.insert(in2);
	itree.insert(in3);

	tree.remove(n1);
	tree.dbg_verify();
	itree.remove(in1);
	itree.dbg_verify();

	tree.remove(n3);
	tree.dbg_verify();
	itree.remove(in3);
	itree.dbg_verify();

	tree.remove(n2);
	tree.dbg_verify();
	itree.remove(in2);
	itree.dbg_verify();

	tree.remove(n0);
	tree.dbg_verify();
	itree.remove(in0);
	itree.dbg_verify();
}

TEST(ZipTreeTest, InsertionAndIterationTest)
{
	ExplicitRankTree tree;
	ImplicitRankTree itree;

	Node nodes[ZIPTREE_TESTSIZE];
	HashRankNode inodes[ZIPTREE_TESTSIZE];

	std::vector<size_t> indices;

	for (size_t i = 0; i < ZIPTREE_TESTSIZE; ++i) {
		nodes[i] = Node((int)i, (int)i);
		inodes[i].set_from(HashRankNode((int)i));

		indices.push_back(i);
	}

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(ZIPTREE_SEED));

	for (auto index : indices) {
		tree.insert(nodes[index]);
		itree.insert(inodes[index]);
		tree.dbg_verify();
		itree.dbg_verify();
	}

	int i = 0;
	for (auto & node : tree) {
		ASSERT_EQ(node.get_data(), i);
		ASSERT_EQ(node.get_rank(), i);
		i++;
	}

	i = 0;
	for (auto & node : itree) {
		ASSERT_EQ(node.get_data(), i);
		i++;
	}
}

TEST(ZipTreeTest, InsertionAndDeletionTest)
{
	ExplicitRankTree tree;
	ImplicitRankTree itree;

	Node nodes[ZIPTREE_TESTSIZE];
	HashRankNode inodes[ZIPTREE_TESTSIZE];

	std::vector<size_t> indices;
	std::vector<size_t> remove_indices;

	for (size_t i = 0; i < ZIPTREE_TESTSIZE; ++i) {
		nodes[i] = Node((int)i, (int)i);
		inodes[i].set_from(HashRankNode((int)i));

		indices.push_back(i);
		remove_indices.push_back(i);
	}

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(ZIPTREE_SEED));
	std::shuffle(remove_indices.begin(), remove_indices.end(),
	             ygg::testing::utilities::Randomizer(ZIPTREE_SEED + 1));

	size_t i = 0;
	for (auto index : indices) {
		ASSERT_EQ(i, tree.size());
		tree.insert(nodes[index]);
		itree.insert(inodes[index]);
		i++;
	}

	tree.dbg_verify();
	itree.dbg_verify();

	for (auto index : remove_indices) {
		if (index % 2 == 0) {
			tree.remove(nodes[index]);
			itree.remove(inodes[index]);
		} else {
			tree.erase(nodes[index].data);
			itree.erase(inodes[index].data);
		}

		i--;
		ASSERT_EQ(i, tree.size());

		tree.dbg_verify();
		itree.dbg_verify();
	}

	ASSERT_TRUE(tree.empty());
	auto it = tree.begin();
	ASSERT_TRUE(it == tree.end());
	auto iit = itree.begin();
	ASSERT_TRUE(iit == itree.end());
}

TEST(ZipTreeTest, EraseIteratorTest)
{
	ExplicitRankTree tree;

	Node n1;
	n1.data = 0;
	tree.insert(n1);

	Node n2;
	n2.data = 1;
	tree.insert(n2);

	ASSERT_FALSE(tree.empty());
	tree.dbg_verify();

	auto it = tree.begin();
	Node * removed_node = tree.erase(it);
	ASSERT_EQ(removed_node, &n1);

	ASSERT_EQ(tree.find(0), tree.end());
}

TEST(ZipTreeTest, EraseIteratorSTLReturnTest)
{
	using MyNode = NodeBase<TreeFlags::STL_ERASE>;
	auto tree = ExplicitRankTreeBase<TreeFlags::STL_ERASE>();

	MyNode n1;
	n1.data = 0;
	tree.insert(n1);

	MyNode n2;
	n2.data = 1;
	tree.insert(n2);

	ASSERT_FALSE(tree.empty());
	tree.dbg_verify();

	auto it = tree.begin();
	auto next_it = tree.erase(it);
	ASSERT_EQ(next_it, tree.begin());

	ASSERT_EQ(tree.find(0), tree.end());
}

TEST(ZipTreeTest, EraseIteratorSTLAllTest)
{
	using MyNode = NodeBase<TreeFlags::STL_ERASE>;
	auto tree = ExplicitRankTreeBase<TreeFlags::STL_ERASE>();

	std::vector<MyNode> zero_nodes(10);
	for (auto & node : zero_nodes) {
		node.data = 0;
	}
	std::vector<MyNode> one_nodes(10);
	for (auto & node : one_nodes) {
		node.data = 1;
	}
	std::vector<MyNode> two_nodes(10);
	for (auto & node : two_nodes) {
		node.data = 2;
	}

	for (unsigned int i = 0; i < 10; ++i) {
		tree.insert(zero_nodes[i]);
		tree.insert(one_nodes[i]);
		tree.insert(two_nodes[i]);
	}

	size_t erased_count = tree.erase(1);
	ASSERT_EQ(erased_count, 10);
	ASSERT_EQ(tree.find(1), tree.end());
	ASSERT_EQ(tree.size(), 20);
}

TEST(ZipTreeTest, ComprehensiveTest)
{
	ExplicitRankTree tree;
	ImplicitRankTree itree;

	Node persistent_nodes[ZIPTREE_TESTSIZE];
	HashRankNode ipersistent_nodes[ZIPTREE_TESTSIZE];

	std::vector<unsigned int> indices;
	std::mt19937 rng(ZIPTREE_SEED);

	std::set<unsigned int> values_seen;

	for (unsigned int i = 0; i < ZIPTREE_TESTSIZE; ++i) {
		unsigned int data = 10 * i;
		persistent_nodes[i] = Node((int)data, (int)data);
		ipersistent_nodes[i].set_from(HashRankNode((int)data));

		indices.push_back(i);
		values_seen.insert(data);
	}

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(ZIPTREE_SEED));

	for (auto index : indices) {
		tree.insert(persistent_nodes[index]);
		itree.insert(ipersistent_nodes[index]);
	}

	tree.dbg_verify();
	itree.dbg_verify();

	Node transient_nodes[ZIPTREE_TESTSIZE];
	HashRankNode itransient_nodes[ZIPTREE_TESTSIZE];

	for (unsigned int i = 0; i < ZIPTREE_TESTSIZE; ++i) {
		std::uniform_int_distribution<unsigned int> uni(
		    0, 10 * (ZIPTREE_TESTSIZE + 1));
		unsigned int data = uni(rng);

		while (values_seen.find(data) != values_seen.end()) {
			data = uni(rng);
		}

		transient_nodes[i] = Node((int)data, (int)data);
		itransient_nodes[i].set_from(HashRankNode((int)data));

		values_seen.insert(data);
	}

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(ZIPTREE_SEED));

	for (auto index : indices) {
		tree.insert(transient_nodes[index]);
		itree.insert(itransient_nodes[index]);
	}

	tree.dbg_verify();
	itree.dbg_verify();

	for (int i = 0; i < (int)ZIPTREE_TESTSIZE; ++i) {
		tree.remove(transient_nodes[i]);
		itree.remove(itransient_nodes[i]);

		tree.dbg_verify();
		itree.dbg_verify();
	}

	// Query elements
	for (int i = 0; i < (int)ZIPTREE_TESTSIZE; ++i) {
		auto it = tree.find(persistent_nodes[i]);
		assert(&(*it) == &(persistent_nodes[i]));
		ASSERT_EQ(&(*it), &(persistent_nodes[i]));
	}
}

/*****************************************
 * Test for individual bugs
 *****************************************/
TEST(ZipTreeTest, Bug1Test)
{

	ImplicitRankTree itree;

	HashRankNode nodes[] = {{40}, {0},  {10}, {30}, {20},
	                        {47}, {26}, {4},  {19}, {13}};

	for (auto & node : nodes) {
		itree.insert(node);
		itree.dbg_verify();
	}

	itree.remove(nodes[7]); // 4
	itree.dbg_verify();
	itree.remove(nodes[9]); // 13
	itree.dbg_verify();
	itree.remove(nodes[5]); // 47
	itree.dbg_verify();
	itree.remove(nodes[8]); // 19
	itree.dbg_verify();
}

} // namespace ziptree
} // namespace testing
} // namespace ygg

#endif // TEST_ZIPTREE_HPP
