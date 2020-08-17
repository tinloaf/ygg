#ifndef TEST_INTERVALTREE_HPP
#define TEST_INTERVALTREE_HPP

#include "../src/intervaltree.hpp"
#include "randomizer.hpp"

#include <unordered_set>

namespace ygg {
namespace testing {
namespace intervaltree {

// necessary to build a unordered set of pairs.
// Since both values will always come from a random distribution anyways, we
// don't need a good hash function here
struct pair_hash
{
	inline std::size_t
	operator()(const std::pair<unsigned int, unsigned int> & v) const
	{
		return v.first * 31 + v.second;
	}
};

using namespace ygg;

constexpr int IT_TESTSIZE = 1500;

using Interval = std::pair<unsigned int, unsigned int>;

template <class Node>
class MyNodeTraits : public ITreeNodeTraits<Node> {
public:
	using key_type = unsigned int;

	static unsigned int
	get_lower(const Node & node)
	{
		return node.lower;
	}

	static unsigned int
	get_upper(const Node & node)
	{
		return node.upper;
	}

	static unsigned int
	get_lower(const Interval & i)
	{
		return std::get<0>(i);
	}

	static unsigned int
	get_upper(const Interval & i)
	{
		return std::get<1>(i);
	}

	static std::string
	get_id(const Node * node)
	{
		return std::to_string(node->data);
	}
};

class ITNode : public ITreeNodeBase<ITNode, MyNodeTraits<ITNode>> {
public:
	int data;
	unsigned int lower;
	unsigned int upper;

	ITNode() : data(0), lower(0), upper(0){};
	explicit ITNode(unsigned int lower_in, unsigned int upper_in, int data_in)
	    : data(data_in), lower(lower_in), upper(upper_in){};
	ITNode(const ITNode & other)
	    : data(other.data), lower(other.lower), upper(other.upper){};
	ITNode & operator=(const ITNode & other) = default;
};

template <class Options>
class ITNodeOpt
    : public ITreeNodeBase<ITNodeOpt<Options>, MyNodeTraits<ITNodeOpt<Options>>,
                           Options> {
public:
	int data;
	unsigned int lower;
	unsigned int upper;

	ITNodeOpt() : data(0), lower(0), upper(0){};
	explicit ITNodeOpt(unsigned int lower_in, unsigned int upper_in, int data_in)
	    : data(data_in), lower(lower_in), upper(upper_in){};
	ITNodeOpt(const ITNodeOpt<Options> & other)
	    : data(other.data), lower(other.lower), upper(other.upper){};
	ITNodeOpt<Options> & operator=(const ITNodeOpt<Options> & other) = default;
};

TEST(ITreeTest, TrivialInsertionTest)
{
	auto tree = IntervalTree<ITNode, MyNodeTraits<ITNode>>();

	ASSERT_TRUE(tree.empty());
	ITNode n(0, 10, 0);
	tree.insert(n);
	ASSERT_FALSE(tree.empty());

	ASSERT_TRUE(tree.verify_integrity());
}

TEST(ITreeTest, CatchBug3)
{
	auto t = IntervalTree<ITNode, MyNodeTraits<ITNode>>();
	ITNode nodes[5];
	for (std::uint64_t i = 0; i < 5; i++) {
		nodes[i].lower = static_cast<unsigned int>(i);
		nodes[i].upper = static_cast<unsigned int>(i) + 5;
		nodes[i].data = static_cast<int>(i);
		t.insert(nodes[i]);
	}

	t.verify_integrity();

	Interval query_range{0, 0};
	size_t counter = 0;
	auto query_result = t.query(query_range);
	for (auto it = query_result.begin(); it != query_result.end(); ++it) {
		const auto & node = *it;
		ASSERT_EQ(node.data, counter);
		counter++;
	}
}

TEST(ITreeTest, RandomInsertionTest)
{
	auto tree = IntervalTree<ITNode, MyNodeTraits<ITNode>>();

	ITNode nodes[IT_TESTSIZE];
	std::mt19937 rng(4); // chosen by fair xkcd

	for (unsigned int i = 0; i < IT_TESTSIZE; ++i) {
		std::uniform_int_distribution<unsigned int> bounds_distr(
		    0, std::numeric_limits<unsigned int>::max() / 2);
		unsigned int lower = bounds_distr(rng);
		unsigned int upper = lower + bounds_distr(rng);

		nodes[i] = ITNode(lower, upper, static_cast<int>(i));

		std::string fname = std::string("/tmp/trees/before-") + std::to_string(i) +
		                    std::string(".dot");
		tree.dump_to_dot(fname);

		tree.insert(nodes[i]);

		fname = std::string("/tmp/trees/after-") + std::to_string(i) +
		        std::string(".dot");
		tree.dump_to_dot(fname);

		ASSERT_TRUE(tree.verify_integrity());
	}
}

TEST(ITreeTest, RandomInsertionRandomDeletionTest)
{
	auto tree = IntervalTree<ITNode, MyNodeTraits<ITNode>>();

	ITNode nodes[IT_TESTSIZE];
	std::vector<unsigned int> indices;
	std::mt19937 rng(4); // chosen by fair xkcd

	for (unsigned int i = 0; i < IT_TESTSIZE; ++i) {
		std::uniform_int_distribution<unsigned int> bounds_distr(
		    0, std::numeric_limits<unsigned int>::max() / 2);
		unsigned int lower = bounds_distr(rng);
		unsigned int upper = lower + bounds_distr(rng);

		nodes[i] = ITNode(lower, upper, static_cast<int>(i));

		tree.insert(nodes[i]);
		indices.push_back(i);
	}

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(4));

	ASSERT_TRUE(tree.verify_integrity());

	for (unsigned int i = 0; i < IT_TESTSIZE; ++i) {
		std::string fname = std::string("/tmp/trees/before-") + std::to_string(i) +
		                    std::string(".dot");
		tree.dump_to_dot(fname);

		tree.remove(nodes[indices[i]]);

		fname = std::string("/tmp/trees/after-") + std::to_string(i) +
		        std::string(".dot");
		tree.dump_to_dot(fname);

		ASSERT_TRUE(tree.verify_integrity());
	}
}

TEST(ITreeTest, TrivialQueryTest)
{
	auto tree = IntervalTree<ITNode, MyNodeTraits<ITNode>>();

	ITNode n(10, 20, 0);
	tree.insert(n);

	ASSERT_TRUE(tree.verify_integrity());

	bool found = false;
	ITNode containing(0, 30, 1);
	for (const auto & node : tree.query(containing)) {
		ASSERT_EQ(&n, &node);
		found = true;
	}
	ASSERT_TRUE(found);

	found = false;
	ITNode contained(11, 19, 1);
	for (const auto & node : tree.query(contained)) {
		ASSERT_EQ(&n, &node);
		found = true;
	}
	ASSERT_TRUE(found);

	found = false;
	ITNode overlap_left(0, 15, 1);
	for (const auto & node : tree.query(overlap_left)) {
		ASSERT_EQ(&n, &node);
		found = true;
	}
	ASSERT_TRUE(found);

	found = false;
	ITNode overlap_right(15, 30, 1);
	for (const auto & node : tree.query(overlap_right)) {
		ASSERT_EQ(&n, &node);
		found = true;
	}
	ASSERT_TRUE(found);

	found = false;
	ITNode right(30, 40, 1);
	for (const auto & node : tree.query(right)) {
		(void)node;
		found = true;
	}
	ASSERT_FALSE(found);

	found = false;
	ITNode left(0, 5, 1);
	for (const auto & node : tree.query(left)) {
		(void)node;
		found = true;
	}
	ASSERT_FALSE(found);
}

TEST(ITreeTest, SimpleQueryTest)
{
	auto tree = IntervalTree<ITNode, MyNodeTraits<ITNode>>();

	ITNode n1(10, 20, 1);
	tree.insert(n1);
	ITNode n2(21, 30, 2);
	tree.insert(n2);
	ITNode n3(15, 25, 3);
	tree.insert(n3);
	ITNode n4(40, 50, 4);
	tree.insert(n4);

	ASSERT_TRUE(tree.verify_integrity());

	// Should find all
	ITNode query_all(0, 60, 0);
	auto container = tree.query(query_all);
	auto it = container.begin();
	ASSERT_EQ(&(*it), &n1);
	it++;
	ASSERT_EQ(&(*it), &n3);
	it++;
	ASSERT_EQ(&(*it), &n2);
	it++;
	ASSERT_EQ(&(*it), &n4);
	it++;
	ASSERT_EQ(it, container.end());

	// Should find n1, n3
	ITNode query_n1n3(0, 15, 0);
	container = tree.query(query_n1n3);
	it = container.begin();
	ASSERT_EQ(&(*it), &n1);
	it++;
	ASSERT_EQ(&(*it), &n3);
	it++;
	ASSERT_EQ(it, container.end());

	// Should find n3, n2
	ITNode query_n3n2(25, 25, 0);
	container = tree.query(query_n3n2);
	it = container.begin();
	ASSERT_EQ(&(*it), &n3);
	it++;
	ASSERT_EQ(&(*it), &n2);
	it++;
	ASSERT_EQ(it, container.end());

	// Should find n4
	ITNode query_n4(50, 60, 0);
	container = tree.query(query_n4);
	it = container.begin();
	ASSERT_EQ(&(*it), &n4);
	it++;
	ASSERT_EQ(it, container.end());

	// Should find nothing
	ITNode query_left(0, 2, 0);
	container = tree.query(query_left);
	it = container.begin();
	ASSERT_EQ(it, container.end());

	// Should find nothing
	ITNode query_right(70, 80, 0);
	container = tree.query(query_right);
	it = container.begin();
	ASSERT_EQ(it, container.end());
}

TEST(ITreeTest, ComprehensiveTest)
{
	auto tree = IntervalTree<ITNode, MyNodeTraits<ITNode>>();

	ITNode persistent_nodes[IT_TESTSIZE];
	std::vector<unsigned int> indices;
	std::mt19937 rng(4); // chosen by fair xkcd

	for (unsigned int i = 0; i < IT_TESTSIZE; ++i) {
		unsigned int lower = 10 * i;
		unsigned int upper = lower + 50;

		persistent_nodes[i] = ITNode(lower, upper, static_cast<int>(i));
		indices.push_back(i);
	}

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(4));

	for (auto index : indices) {
		tree.insert(persistent_nodes[index]);
	}

	ASSERT_TRUE(tree.verify_integrity());

	ITNode transient_nodes[IT_TESTSIZE];
	for (unsigned int i = 0; i < IT_TESTSIZE; ++i) {
		std::uniform_int_distribution<unsigned int> bounds_distr(
		    0, 10 * IT_TESTSIZE / 2);
		unsigned int lower = bounds_distr(rng);
		unsigned int upper = lower + bounds_distr(rng);

		transient_nodes[i] =
		    ITNode(lower, upper, static_cast<int>(IT_TESTSIZE + i));
	}

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(4));

	for (auto index : indices) {
		tree.insert(transient_nodes[index]);
	}

	ASSERT_TRUE(tree.verify_integrity());

	for (int i = 0; i < IT_TESTSIZE; ++i) {
		tree.remove(transient_nodes[i]);
	}

	ASSERT_TRUE(tree.verify_integrity());

	// std::string fname = std::string("/tmp/trees/comprehensive.dot");
	// tree.dump_to_dot(fname);

	// Query prefixes
	for (int i = 0; i < IT_TESTSIZE; ++i) {
		ITNode query(0, static_cast<unsigned int>((i + 1) * 10) - 1, 0);
		auto container = tree.query(query);
		auto it = container.begin();

		for (int expected = 0; expected <= i; ++expected) {
			assert(&(*it) == &(persistent_nodes[expected]));
			ASSERT_EQ(&(*it), &(persistent_nodes[expected]));
			++it;
		}
		assert(it == container.end());
		ASSERT_EQ(it, container.end());
	}
}

TEST(ITreeTest, RandomEqualInsertionRandomDeletionTest)
{
	auto tree = IntervalTree<ITNode, MyNodeTraits<ITNode>>();

	ITNode nodes[IT_TESTSIZE * 5];
	std::vector<unsigned int> indices;
	std::mt19937 rng(4); // chosen by fair xkcd

	for (unsigned int i = 0; i < IT_TESTSIZE; ++i) {
		std::uniform_int_distribution<unsigned int> bounds_distr(
		    0, std::numeric_limits<unsigned int>::max() / 2);
		unsigned int lower = bounds_distr(rng);
		unsigned int upper = lower + bounds_distr(rng);

		for (unsigned int j = 0; j < 5; ++j) {
			nodes[5 * i + j] = ITNode(lower, upper, static_cast<int>(5 * i + j));
			tree.insert(nodes[5 * i + j]);
			indices.push_back(5 * i + j);
		}
	}

	std::shuffle(indices.begin(), indices.end(),
	             ygg::testing::utilities::Randomizer(4));

	ASSERT_TRUE(tree.verify_integrity());

	for (unsigned int i = 0; i < 5 * IT_TESTSIZE; ++i) {
		tree.remove(nodes[indices[i]]);
		ASSERT_TRUE(tree.verify_integrity());
	}
}

TEST(ITreeTest, SlowFindTest)
{
	auto tree = IntervalTree<ITNode, MyNodeTraits<ITNode>>();

	ITNode nodes[IT_TESTSIZE];
	std::mt19937 rng(4); // chosen by fair xkcd
	std::unordered_set<std::pair<unsigned int, unsigned int>, pair_hash> seen;

	for (unsigned int i = 0; i < IT_TESTSIZE; ++i) {
		std::uniform_int_distribution<unsigned int> bounds_distr(
		    0, std::numeric_limits<unsigned int>::max() / 2);
		unsigned int lower = bounds_distr(rng);
		unsigned int upper = lower + bounds_distr(rng);
		nodes[i] = ITNode(lower, upper, i);
		seen.emplace(lower, upper);
		tree.insert(nodes[i]);
	}

	ITNode query_nodes[IT_TESTSIZE];
	for (unsigned int i = 0; i < IT_TESTSIZE;) {
		std::uniform_int_distribution<unsigned int> bounds_distr(
		    0, std::numeric_limits<unsigned int>::max() / 2);
		unsigned int lower = bounds_distr(rng);
		unsigned int upper = lower + bounds_distr(rng);

		if (seen.find({lower, upper}) == seen.end()) {
			query_nodes[i] = ITNode(lower, upper, i);
			i++;
		}
	}

	ASSERT_TRUE(tree.verify_integrity());

	for (unsigned int i = 0; i < IT_TESTSIZE; ++i) {
		auto it = tree.find(nodes[i]);
		ASSERT_NE(it, tree.end());
		ASSERT_EQ(it->data, i);

		it = tree.find(query_nodes[i]);
		ASSERT_EQ(it, tree.end());
	}
}

TEST(ITreeTest, FastFindTest)
{
	using Options =
	    ygg::TreeOptions<TreeFlags::MULTIPLE, TreeFlags::CONSTANT_TIME_SIZE,
	                     ygg::TreeFlags::ITREE_FAST_FIND>;
	auto tree = IntervalTree<ITNodeOpt<Options>, MyNodeTraits<ITNodeOpt<Options>>,
	                         Options>();

	ITNodeOpt<Options> nodes[IT_TESTSIZE];
	std::mt19937 rng(4); // chosen by fair xkcd
	std::unordered_set<std::pair<unsigned int, unsigned int>, pair_hash> seen;

	for (unsigned int i = 0; i < IT_TESTSIZE; ++i) {
		std::uniform_int_distribution<unsigned int> bounds_distr(
		    0, std::numeric_limits<unsigned int>::max() / 2);
		unsigned int lower = bounds_distr(rng);
		unsigned int upper = lower + bounds_distr(rng);
		nodes[i] = ITNodeOpt<Options>(lower, upper, i);
		seen.emplace(lower, upper);
		tree.insert(nodes[i]);
	}

	ITNodeOpt<Options> query_nodes[IT_TESTSIZE];
	for (unsigned int i = 0; i < IT_TESTSIZE;) {
		std::uniform_int_distribution<unsigned int> bounds_distr(
		    0, std::numeric_limits<unsigned int>::max() / 2);
		unsigned int lower = bounds_distr(rng);
		unsigned int upper = lower + bounds_distr(rng);

		if (seen.find({lower, upper}) == seen.end()) {
			query_nodes[i] = ITNodeOpt<Options>(lower, upper, i);
			i++;
		}
	}

	ASSERT_TRUE(tree.verify_integrity());

	for (unsigned int i = 0; i < IT_TESTSIZE; ++i) {
		auto it = tree.find(nodes[i]);
		ASSERT_NE(it, tree.end());
		ASSERT_EQ(it->data, i);

		it = tree.find(query_nodes[i]);
		ASSERT_EQ(it, tree.end());
	}
}

} // namespace intervaltree
} // namespace testing
} // namespace ygg

#endif // TEST_INTERVALTREE_HPP
