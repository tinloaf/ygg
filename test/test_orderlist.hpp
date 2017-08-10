//
// Created by lukas on 09.08.17.
//

#ifndef YGG_TEST_ORDERLIST_HPP
#define YGG_TEST_ORDERLIST_HPP

#include "../src/orderlist.hpp"

#define TESTSIZE 1000

using namespace ygg;
using namespace ygg::utilities;

class ONode : public OrderListItem<ONode, true> {
public:
	ONode * _rbt_next;
	ONode * _rbt_prev;

	ONode () : _rbt_next(nullptr), _rbt_prev(nullptr) {};
};

using OLI = OrderListItem<ONode, true>;

TEST(OListTest, TrivialInsertionTest) {
	ONode n;
	OLI::insert_between(n, nullptr, nullptr);
	ASSERT_EQ(n._rbt_point, -1);
}

TEST(OListTest, LinearInsertionTest) {
	std::vector<ONode> n(TESTSIZE);
	for (auto i = 0 ; i < TESTSIZE ; ++i) {
		if (i > 0) {
			n[i]._rbt_prev = &(n[i-1]);
		} else {
			n[i]._rbt_prev = nullptr;
		}
		if (i < TESTSIZE - 1) {
			n[i]._rbt_next = &(n[i+1]);
		} else {
			n[i]._rbt_next = nullptr;
		}
	}

	// Insert linearly
	for (auto i = 0 ; i < TESTSIZE ; ++i) {
		ONode * prev = nullptr;
		ONode * next = nullptr;
		if (i > 0) {
			prev = &n[i-1];
		}
		if (i < TESTSIZE - 1) {
			next = &n[i+1];
		}

		OLI::insert_between(n[i], prev, next);
	}

	for (auto i = 0 ; i < TESTSIZE - 1; ++i) {
		ASSERT_TRUE(n[i]._rbt_point < n[i + 1]._rbt_point);
	}
}

TEST(OListTest, NestedInsertionTest) {
	std::vector<ONode> n(TESTSIZE);
	for (auto i = 0 ; i < TESTSIZE ; ++i) {
		if (i > 0) {
			n[i]._rbt_prev = &(n[i-1]);
		} else {
			n[i]._rbt_prev = nullptr;
		}
		if (i < TESTSIZE - 1) {
			n[i]._rbt_next = &(n[i+1]);
		} else {
			n[i]._rbt_next = nullptr;
		}
	}

	// Insert first
	OLI::insert_between(n[0], nullptr, nullptr);
	// Insert last
	OLI::insert_between(n[TESTSIZE-1], &n[0], nullptr);

	// insert between
	for (auto i = 1 ; i < TESTSIZE-1 ; ++i) {
		ONode * prev = &n[i-1];

		OLI::insert_between(n[i], prev, &n[TESTSIZE-1]);
	}

	for (auto i = 0 ; i < TESTSIZE - 1; ++i) {
		ASSERT_TRUE(n[i]._rbt_point < n[i + 1]._rbt_point);
	}
}

#endif //YGG_TEST_ORDERLIST_HPP
