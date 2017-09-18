//
// Created by lukas on 09.08.17.
//

#ifndef YGG_TEST_LIST_HPP
#define YGG_TEST_LIST_HPP

#include "../src/list.hpp"

#define LIST_TESTSIZE 1000

using namespace ygg;
using namespace ygg::utilities;

class LNode : public ListNodeBase<LNode> {
public:
	int data;
};

using MyList = List<LNode>;

TEST(ListTest, SimpleTest) {
	LNode n1, n2, n3;
	n1.data = 1;
	n2.data = 2;
	n3.data = 3;

	MyList l;

	l.insert(nullptr, &n3);
	l.insert(&n3, &n1);
	l.insert(&n3, &n2);

	auto it = l.begin();
	ASSERT_EQ(it->data, 1);
	++it;
	ASSERT_EQ(it->data, 2);
	it++;
	ASSERT_EQ(it->data, 3);
	++it;
	ASSERT_EQ(it, l.end());
}

TEST(ListTest, ComprehensiveTest) {
	LNode delete_me[LIST_TESTSIZE];
	LNode nodes[LIST_TESTSIZE];
	MyList l;

	for (unsigned int i = 0 ; i < LIST_TESTSIZE ; ++i) {
		delete_me[i].data = 0;
		nodes[i].data = i;

		l.insert(nullptr, &delete_me[i]);
	}

	for (unsigned int i = 0 ; i < LIST_TESTSIZE ; ++i) {
		l.insert(&delete_me[i], &nodes[i]);
	}

	for (unsigned int i = 0 ; i < LIST_TESTSIZE ; ++i) {
		l.remove(&delete_me[i]);
	}

	unsigned int i = 0;
	for (auto & n : l) {
		ASSERT_EQ(n.data, i++);
	}
}

#endif //YGG_TEST_LIST_HPP
