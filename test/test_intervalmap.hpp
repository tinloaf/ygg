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

#define IMAP_TESTSIZE 20
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
}

TEST(IMapTest, SimpleInsertionTest)
{
	IMap m;

	Node n;
	n.lower = 1;
	n.upper = 2;
	n.value = 10;

	m.insert(n);
	m.dbg_verify();

	auto it = m.begin();
	ASSERT_EQ(it.get_lower(), 1);
	ASSERT_EQ(it.get_upper(), 2);
	++it;
	ASSERT_EQ(it, m.end());

	m.remove(n);
	m.dbg_verify();

	it = m.begin();
	ASSERT_EQ(it, m.end());

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
		std::cout << "############################ Deleting ############################\n";
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
