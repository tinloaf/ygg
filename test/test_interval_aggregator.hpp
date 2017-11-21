//
// Created by lukas on 05.09.17.
//

#ifndef YGG_TEST_INTERVAL_AGGREGATOR_HPP
#define YGG_TEST_INTERVAL_AGGREGATOR_HPP

#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <algorithm>

#include "../src/ygg.hpp"

#define IAGG_TESTSIZE 2000

namespace test_interval_agg {
using namespace ygg;

class Node : public DynSegTreeNodeBase<int, int, int> {
public:
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


using IAgg = DynamicSegmentTree<Node, NodeTraits>;

TEST(IAggTest, TrivialTest)
{
	Node n;
	n.lower = 2;
	n.upper = 5;
	n.value = 10;

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

}

#endif //YGG_TEST_INTERVAL_AGGREGATOR_HPP
