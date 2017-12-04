//
// Created by lukas on 05.09.17.
//

#ifndef YGG_TEST_INTERVAL_AGGREGATOR_HPP
#define YGG_TEST_INTERVAL_AGGREGATOR_HPP

#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <algorithm>

#include <boost/icl/interval_map.hpp>

#include "../src/ygg.hpp"

#define IAGG_TESTSIZE 2000
#define IAGG_DELETION_TESTSIZE 500
#define IAGG_DELETION_ITERATIONS 100

namespace test_interval_agg {
using namespace boost::icl;
using namespace ygg;

using Combiners = CombinerPack<int, MaxCombiner<int>>;

class Node : public DynSegTreeNodeBase<int, int, int, Combiners> {
public:
	Node(int lower_in, int upper_in, int value_in) : lower(lower_in), upper(upper_in),
	                                                 value(value_in) {};
	Node () = default;
	int lower;
	int upper;
	int value;
};

class NodeTraits : public DynSegTreeNodeTraits<Node> {
public:
	using key_type = int;
	using value_type = int;

	static key_type get_lower(const Node & n) {
		return n.lower;
	}

	static key_type get_upper(const Node & n) {
		return n.upper;
	}

	static value_type get_value(const Node & n) {
		return n.value;
	}
};


using IAgg = DynamicSegmentTree<Node, NodeTraits, Combiners>;

TEST(IAggTest, TrivialTest)
{
	Node n(2,5,10);

	IAgg agg;
	agg.insert(n);

	int agg_val = agg.query(3);
	ASSERT_EQ(agg_val, 10);
}

TEST(IAggTest, NestingTest)
{
	Node n[IAGG_TESTSIZE];
	for (unsigned int i = 0 ; i < IAGG_TESTSIZE ; ++i) {
		n[i].lower = i;
		n[i].upper = 2 * IAGG_TESTSIZE - i + 1;
		n[i].value = 1;
	}

	IAgg agg;

	for (unsigned int i = 0 ; i < IAGG_TESTSIZE ; ++i) {
		agg.insert(n[i]);
	}

	for (unsigned int i = 0 ; i < IAGG_TESTSIZE ; ++i) {
		auto val = agg.query(i);
		ASSERT_EQ(val, i + 1);
	}
}

TEST(IAggTest, OverlappingTest)
{
	Node n[IAGG_TESTSIZE];
	for (unsigned int i = 0 ; i < IAGG_TESTSIZE ; ++i) {
		n[i].lower = i;
		n[i].upper = IAGG_TESTSIZE + i;
		n[i].value = 1;
	}

	IAgg agg;

	for (unsigned int i = 0 ; i < IAGG_TESTSIZE ; ++i) {
		agg.insert(n[i]);
	}

	for (unsigned int i = 0 ; i < IAGG_TESTSIZE ; ++i) {
		auto val = agg.query(i);
		ASSERT_EQ(val, i + 1);
	}
	for (unsigned int i = 0 ; i < IAGG_TESTSIZE ; ++i) {
		auto val = agg.query(i + IAGG_TESTSIZE);
		ASSERT_EQ(val, IAGG_TESTSIZE - i - 1);
	}
}

TEST(IAggTest, DeletionTest)
{
	Node n[IAGG_DELETION_TESTSIZE];
	for (unsigned int i = 0 ; i < IAGG_DELETION_TESTSIZE ; ++i) {
		n[i].lower = i;
		n[i].upper = IAGG_DELETION_TESTSIZE + i;
		n[i].value = 1;
	}

	IAgg agg;

	for (unsigned int i = 0 ; i < IAGG_DELETION_TESTSIZE ; ++i) {
		agg.insert(n[i]);
	}

	std::mt19937 rng(5); // chosen by fair xkcd plus one

	for (unsigned int j = 0 ; j < IAGG_DELETION_ITERATIONS ; ++j) {
		//std::cout << "\n\n\n=================================================\n\n\n";
		std::uniform_int_distribution<unsigned int> bounds_distr(0, 2 * IAGG_DELETION_TESTSIZE);
		unsigned int lower = bounds_distr(rng);
		unsigned int upper = lower + bounds_distr(rng) + 1;

		Node deleteme(lower, upper, 42);
		//std::cout << "Test interval: " << lower << "->" << upper << "\n\n";

		//std::cout << "================= INSERT ====================\n";

		agg.insert(deleteme);

		for (unsigned int i = 0 ; i < IAGG_DELETION_TESTSIZE ; ++i) {
			auto val = agg.query(i);
			if ((i >= lower) && (i < upper)) {
				ASSERT_EQ(val, i + 1 + 42);
			} else {
				ASSERT_EQ(val, i + 1);
			}
		}
		for (unsigned int i = 0 ; i < IAGG_DELETION_TESTSIZE ; ++i) {
			auto val = agg.query(i + IAGG_DELETION_TESTSIZE);
				if ((i  + IAGG_DELETION_TESTSIZE >= lower) && (i  + IAGG_DELETION_TESTSIZE < upper)) {
					ASSERT_EQ(val, IAGG_DELETION_TESTSIZE - i - 1 + 42);
				} else {
					ASSERT_EQ(val, IAGG_DELETION_TESTSIZE - i - 1);
				}
		}

		//std::cout << "================= REMOVE ====================\n";

		agg.remove(deleteme);

		for (unsigned int i = 0 ; i < IAGG_DELETION_TESTSIZE ; ++i) {
			auto val = agg.query(i);
			ASSERT_EQ(val, i + 1);
		}
		for (unsigned int i = 0 ; i < IAGG_DELETION_TESTSIZE ; ++i) {
			auto val = agg.query(i + IAGG_DELETION_TESTSIZE);
			ASSERT_EQ(val, IAGG_DELETION_TESTSIZE - i - 1);
		}
	}
}

TEST(IAggTest, NestingTestInsertionOverlappingDeletionTest)
{
	Node n[IAGG_TESTSIZE];
	for (unsigned int i = 0 ; i < IAGG_TESTSIZE ; ++i) {
		n[i].lower = i;
		n[i].upper = 2 * IAGG_TESTSIZE - i + 1;
		n[i].value = 1;
	}

	Node transient[IAGG_TESTSIZE];
	for (unsigned int i = 0 ; i < IAGG_TESTSIZE ; ++i) {
		transient[i].lower = i;
		transient[i].upper = IAGG_TESTSIZE + i;
		transient[i].value = 10;
	}

	IAgg agg;

	for (unsigned int i = 0 ; i < IAGG_TESTSIZE ; ++i) {
		agg.insert(n[i]);
		agg.insert(transient[IAGG_TESTSIZE - i - 1]);
	}

	auto val = agg.query(1);
	ASSERT_EQ(val, 22);

	agg.remove(transient[0]);
	val = agg.query(1);
	ASSERT_EQ(val, 12);
	//std::cout << "\n\n###################################################\n\n";
	agg.remove(transient[1]);
	val = agg.query(1);
	ASSERT_EQ(val, 2);

	for (unsigned int i = 2 ; i < IAGG_TESTSIZE ; ++i) {
		agg.remove(transient[i]);
	}

	for (unsigned int i = 0 ; i < IAGG_TESTSIZE ; ++i) {
		auto val = agg.query(i);
		ASSERT_EQ(val, i + 1);
	}
}

TEST(IAggTest, ManyEqualTest)
{
	Node n_middle[IAGG_TESTSIZE];
	for (unsigned int i = 0 ; i < IAGG_TESTSIZE ; ++i) {
		n_middle[i].lower = 10;
		n_middle[i].upper = 20;
		n_middle[i].value = 1;
	}

	Node n_left[IAGG_TESTSIZE];
	for (unsigned int i = 0 ; i < IAGG_TESTSIZE ; ++i) {
		n_left[i].lower = 0;
		n_left[i].upper = 15;
		n_left[i].value = 7;
	}

	Node n_right[IAGG_TESTSIZE];
	for (unsigned int i = 0 ; i < IAGG_TESTSIZE ; ++i) {
		n_right[i].lower = 15;
		n_right[i].upper = 25;
		n_right[i].value = 29;
	}


	IAgg agg;

	for (unsigned int i = 0 ; i < IAGG_TESTSIZE ; ++i) {
		agg.insert(n_middle[i]);
		agg.insert(n_left[i]);
		agg.insert(n_right[i]);
	}

	auto val = agg.query(0);
	ASSERT_EQ(val, IAGG_TESTSIZE * 7);
	val = agg.query(5);
	ASSERT_EQ(val, IAGG_TESTSIZE * 7);
	val = agg.query(10);
	ASSERT_EQ(val, IAGG_TESTSIZE * 7 + IAGG_TESTSIZE * 1);
	val = agg.query(12);
	ASSERT_EQ(val, IAGG_TESTSIZE * 7 + IAGG_TESTSIZE * 1);
	val = agg.query(15);
	ASSERT_EQ(val, IAGG_TESTSIZE * 1 + IAGG_TESTSIZE * 29);
	val = agg.query(17);
	ASSERT_EQ(val, IAGG_TESTSIZE * 1 + IAGG_TESTSIZE * 29);
	val = agg.query(20);
	ASSERT_EQ(val, IAGG_TESTSIZE * 29);
	val = agg.query(22);
	ASSERT_EQ(val, IAGG_TESTSIZE * 29);
	val = agg.query(25);
	ASSERT_EQ(val, 0);
}

TEST(IAggTest, ComprehensiveTest)
{
	Node persistent_nodes[IAGG_TESTSIZE];
	std::vector<unsigned int> indices;
	std::mt19937 rng(4); // chosen by fair xkcd

	IAgg agg;

	for (unsigned int i = 0; i < IAGG_TESTSIZE; ++i) {
		std::uniform_int_distribution<unsigned int> bounds_distr(0, 10 * IAGG_TESTSIZE / 2);
		unsigned int lower = bounds_distr(rng);
		unsigned int upper = lower + 1 + bounds_distr(rng);

		persistent_nodes[i] = Node(lower, upper, i);
		indices.push_back(i);
	}

	Node transient_nodes[IAGG_TESTSIZE];
	for (unsigned int i = 0; i < IAGG_TESTSIZE; ++i) {
		std::uniform_int_distribution<unsigned int> bounds_distr(0, 10 * IAGG_TESTSIZE / 2);
		unsigned int lower = bounds_distr(rng);
		unsigned int upper = lower + 1 + bounds_distr(rng);

		transient_nodes[i] = Node(lower, upper, IAGG_TESTSIZE + i);
	}

	std::random_shuffle(indices.begin(), indices.end(), [&](int i) {
		std::uniform_int_distribution<unsigned int> uni(0, i - 1);
		return uni(rng);
	});

	for (auto index : indices) {
		agg.insert(transient_nodes[index]);
	}

	std::random_shuffle(indices.begin(), indices.end(), [&](int i) {
		std::uniform_int_distribution<unsigned int> uni(0, i - 1);
		return uni(rng);
	});

	for (auto index : indices) {
		agg.insert(persistent_nodes[index]);
	}

	std::random_shuffle(indices.begin(), indices.end(), [&](int i) {
		std::uniform_int_distribution<unsigned int> uni(0, i - 1);
		return uni(rng);
	});

	for (auto index : indices) {
		agg.remove(transient_nodes[index]);
	}

	// Reference data structure
	using BoostMap = interval_map<int, int>;
	BoostMap reference;
	for (auto node : persistent_nodes) {
		reference += std::make_pair(interval<int>::right_open(node.lower, node.upper), node.value);
	}

	auto it = reference.begin();
	while (it != reference.end()) {
		int lower = it->first.lower();
		int upper = it->first.upper();
		int val = it->second;

		for (int i = lower ; i < upper ; ++i) {
			int result = agg.query(i);
			ASSERT_EQ(val, result);
		}

		++it;
	}
}

}

#endif //YGG_TEST_INTERVAL_AGGREGATOR_HPP
