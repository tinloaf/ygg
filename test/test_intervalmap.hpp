//
// Created by lukas on 05.09.17.
//

#ifndef YGG_TEST_INTERVALMAP_HPP
#define YGG_TEST_INTERVALMAP_HPP

#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <algorithm>

#include "../src/ygg.hpp"

#define IMAP_TESTSIZE 1000
#define IMAP_MULTIPLICITY 3

// TODO test all the callbacks

namespace test_intervalmap {
using namespace ygg;

class Node : public IMapNodeBase<int, int> {
public:
	int lower;
	int upper;
	int value;
};

class NodeTraits : public IMapNodeTraits<Node> {
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


using IMap = IntervalMap<Node, NodeTraits>;

TEST(IMapTest, TrivialTest)
{
	IMap m;
	ASSERT_TRUE(m.empty());
}

TEST(IMapTest, SimpleInsertionTest)
{
	IMap m;
	ASSERT_TRUE(m.empty());

	Node n;
	n.lower = 1;
	n.upper = 2;
	n.value = 10;

	m.insert(n);
	ASSERT_FALSE(m.empty());

	m.dbg_verify();

	auto it = m.begin();
	ASSERT_EQ(it.get_lower(), 1);
	ASSERT_EQ(it.get_upper(), 2);
	++it;
	ASSERT_EQ(it, m.end());
	ASSERT_EQ(m.size(), 1);

	m.remove(n);
	m.dbg_verify();

	it = m.begin();
	ASSERT_EQ(it, m.end());
	ASSERT_EQ(m.size(), 0);
	ASSERT_TRUE(m.empty());
}

TEST(IMapTest, SegmentMergingTest)
{
	std::vector<Node> nodes(IMAP_TESTSIZE);

	IMap m;

	for (unsigned int i = 0 ; i < IMAP_TESTSIZE ; ++i) {
		nodes[i].lower = i;
		nodes[i].upper = i + 1;
		nodes[i].value = 42;
	}

	// Linear insertion
	for (unsigned int i = 0 ; i < IMAP_TESTSIZE ; ++i) {
		m.insert(nodes[i]);
	}
	m.dbg_verify();

	auto it = m.begin();
	ASSERT_EQ(it.get_lower(), 0);
	ASSERT_EQ(it.get_upper(), IMAP_TESTSIZE);
	ASSERT_EQ(it.get_value(), 42);
	++it;
	ASSERT_EQ(it, m.end());

	// Linear deletion
	for (unsigned int i = 0 ; i < IMAP_TESTSIZE ; ++i) {
		m.remove(nodes[i]);
		m.dbg_verify();

		it = m.begin();
		if (i < IMAP_TESTSIZE - 1) {
			ASSERT_EQ(it.get_lower(), i + 1);
		} else {
			ASSERT_EQ(it, m.end());
		}
	}
	m.dbg_verify();

	// Interleaved insertion
	for (unsigned int i = 0 ; i < IMAP_TESTSIZE ; i += 2) {
		m.insert(nodes[i]);
	}
	for (unsigned int i = 1 ; i < IMAP_TESTSIZE ; i += 2) {
		m.insert(nodes[i]);
	}
	m.dbg_verify();

	it = m.begin();
	ASSERT_EQ(it.get_lower(), 0);
	ASSERT_EQ(it.get_upper(), IMAP_TESTSIZE);
	ASSERT_EQ(it.get_value(), 42);
	++it;
	ASSERT_EQ(it, m.end());

	// Interleaved deletion
	for (unsigned int i = 0 ; i < IMAP_TESTSIZE ; i += 2) {
		m.remove(nodes[i]);
		m.dbg_verify();
	}
	for (unsigned int i = 1 ; i < IMAP_TESTSIZE ; i += 2) {
		m.remove(nodes[i]);
		m.dbg_verify();
	}
	m.dbg_verify();
}

TEST(IMapTest, BasedSegmentMergingTest)
{
	std::vector<Node> nodes_small(IMAP_TESTSIZE);
	std::vector<Node> nodes_large(IMAP_TESTSIZE);
	std::vector<Node> nodes_long(IMAP_TESTSIZE);
	std::vector<Node *> insert_me;

	IMap m;

	Node base_left;
	base_left.lower = -10;
	base_left.upper = IMAP_TESTSIZE + 1;
	base_left.value = 1000;

	Node base_right;
	base_right.lower = 0;
	base_right.upper = IMAP_TESTSIZE + 100;
	base_right.value = 3000;

	for (unsigned int i = 0; i < IMAP_TESTSIZE; ++i) {
		nodes_small[i].lower = i;
		nodes_small[i].upper = i + 1;
		nodes_small[i].value = 23;

		nodes_large[i].lower = i;
		nodes_large[i].upper = i + 1;
		nodes_large[i].value = 42;

		nodes_long[i].lower = i;
		nodes_long[i].upper = i + 10;
		nodes_long[i].value = 1;

		insert_me.push_back(&nodes_small[i]);
		insert_me.push_back(&nodes_large[i]);
		insert_me.push_back(&nodes_long[i]);
	}
	insert_me.push_back(&base_left);
	insert_me.push_back(&base_right);

	std::mt19937 rng(4); // chosen by fair xkcd
	std::random_shuffle(insert_me.begin(), insert_me.end(), [&](int i) {
		std::uniform_int_distribution<unsigned int> uni(0, i - 1);
		return uni(rng);
	});

	for (auto n : insert_me) {
		m.insert(*n);
		m.dbg_verify();
	}
	m.dbg_verify();

	auto it = m.begin();
	ASSERT_EQ(it.get_lower(), -10);
	ASSERT_EQ(it.get_upper(), 0);
	ASSERT_EQ(it.get_value(), 1000);
	++it;

	for (unsigned int i = 0 ; i < 9 ; ++i) {
		ASSERT_EQ(it.get_lower(), i);
		ASSERT_EQ(it.get_upper(), i+1);
		ASSERT_EQ(it.get_value(), 1000 + 3000 + 65 + i + 1); // 1000 + 3000 42 + 23 + number of 'long'
																												 // nodes
		++it;
	}

	ASSERT_EQ(it.get_lower(), 9);
	ASSERT_EQ(it.get_upper(), IMAP_TESTSIZE);
	ASSERT_EQ(it.get_value(), 1000 + 3000 + 42 + 23 + 10);
	++it;

	ASSERT_EQ(it.get_lower(), IMAP_TESTSIZE);
	ASSERT_EQ(it.get_upper(), IMAP_TESTSIZE + 1);
	ASSERT_EQ(it.get_value(), 1000 + 3000 + 9);
	++it;

	for (unsigned int i = 1 ; i < 9 ; ++i) {
		ASSERT_EQ(it.get_lower(), IMAP_TESTSIZE + i);
		ASSERT_EQ(it.get_upper(), IMAP_TESTSIZE + i + 1);
		ASSERT_EQ(it.get_value(), 3000 + 10 - i - 1);
		++it;
	}

	ASSERT_EQ(it.get_lower(), IMAP_TESTSIZE + 9);
	ASSERT_EQ(it.get_upper(), IMAP_TESTSIZE + 100);
	ASSERT_EQ(it.get_value(), 3000);
}

TEST(IMapTest, GappedInsertionTest)
{
	IMap m;

	std::vector<Node> nodes(IMAP_TESTSIZE);

	for (unsigned int i = 0 ; i < IMAP_TESTSIZE ; ++i) {
		nodes[i].lower = 3 * i;
		nodes[i].upper = 3 * i + 1;
		nodes[i].value = 42;
	}

	for (unsigned int i = 0 ; i < IMAP_TESTSIZE ; ++i) {
		m.insert(nodes[i]);
		m.dbg_verify();
	}

	auto it = m.begin();
	int i = 0;
	while (it != m.end()) {
		ASSERT_EQ(it.get_lower(), 3 * i);
		ASSERT_EQ(it.get_upper(), 3 * i + 1);
		ASSERT_EQ(it.get_value(), 42);

		++it;

		if (i < IMAP_TESTSIZE - 1) {
			ASSERT_FALSE(it == m.end());
			ASSERT_EQ(it.get_lower(), 3 * i + 1);
			ASSERT_EQ(it.get_upper(), 3 * (i + 1));
			ASSERT_EQ(it.get_value(), 0);

			++i;
			++it;
		}
	}

	ASSERT_EQ(it, m.end());
}

TEST(IMapTest, NestedInsertionTest)
{
	IMap m;

	std::vector<Node> nodes(IMAP_TESTSIZE);

	for (unsigned int i = 0 ; i < IMAP_TESTSIZE ; ++i) {
		nodes[i].lower = i;
		nodes[i].upper = (2 * IMAP_TESTSIZE) - i - 1;
		nodes[i].value = 1;
	}

	for (unsigned int i = 0 ; i < IMAP_TESTSIZE ; ++i) {
		m.insert(nodes[i]);
		m.dbg_verify();
	}

	auto it = m.begin();
	for (unsigned int i = 0 ; i < IMAP_TESTSIZE ; ++i) {
		ASSERT_EQ(it.get_lower(), i);
		ASSERT_EQ(it.get_upper(), i+1);
		ASSERT_EQ(it.get_value(), i+1);
		it++;
	}

	for (unsigned int i = IMAP_TESTSIZE - 1 ; i > 0 ; --i) {
		ASSERT_EQ(it.get_lower(), IMAP_TESTSIZE + (IMAP_TESTSIZE - 1 - i));
		ASSERT_EQ(it.get_upper(), IMAP_TESTSIZE + (IMAP_TESTSIZE - i));
		ASSERT_EQ(it.get_value(), i);
		it++;
	}
}


TEST(IMapTest, MultipleNestedInsertionTest)
{
	IMap m;

	std::vector<Node> nodes(IMAP_MULTIPLICITY * IMAP_TESTSIZE);

	for (unsigned int i = 0 ; i < IMAP_TESTSIZE ; ++i) {
		for (unsigned int j = 0 ; j < IMAP_MULTIPLICITY ; ++j) {
			nodes[i * IMAP_MULTIPLICITY + j].lower = i;
			nodes[i * IMAP_MULTIPLICITY + j].upper = (2 * IMAP_TESTSIZE) - i - 1;
			nodes[i * IMAP_MULTIPLICITY + j].value = 1;
		}
	}

	for (unsigned int i = 0 ; i < (IMAP_MULTIPLICITY * IMAP_TESTSIZE) ; ++i) {
		m.insert(nodes[i]);
		m.dbg_verify();
	}

	auto it = m.begin();
	for (unsigned int i = 0 ; i < IMAP_TESTSIZE ; ++i) {
		ASSERT_EQ(it.get_lower(), i);
		ASSERT_EQ(it.get_upper(), i+1);
		ASSERT_EQ(it.get_value(), (i+1) * IMAP_MULTIPLICITY);
		it++;
	}

	for (unsigned int i = IMAP_TESTSIZE - 1 ; i > 0 ; --i) {
		ASSERT_EQ(it.get_lower(), IMAP_TESTSIZE + (IMAP_TESTSIZE - 1 - i));
		ASSERT_EQ(it.get_upper(), IMAP_TESTSIZE + (IMAP_TESTSIZE - i));
		ASSERT_EQ(it.get_value(), i * IMAP_MULTIPLICITY);
		it++;
	}}
}

#endif //YGG_TEST_INTERVALMAP_HPP
