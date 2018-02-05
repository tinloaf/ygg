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

	ASSERT_TRUE(l.empty());

	l.insert(nullptr, &n3);
	l.insert(&n3, &n1);
	l.insert(&n3, &n2);

	ASSERT_FALSE(l.empty());

	auto it = l.begin();
	ASSERT_EQ(it->data, 1);
	++it;
	ASSERT_EQ(it->data, 2);
	it++;
	ASSERT_EQ(it->data, 3);
	++it;
	ASSERT_EQ(it, l.end());
	ASSERT_EQ(l.size(), 3);

	l.clear();

	ASSERT_TRUE(l.empty());
}

TEST(ListTest, ComprehensiveTest) {
	LNode delete_me[LIST_TESTSIZE];
	LNode nodes[LIST_TESTSIZE];
	MyList l;

	size_t size = 0;
	for (unsigned int i = 0 ; i < LIST_TESTSIZE ; ++i) {
		delete_me[i].data = 0;
		nodes[i].data = i;

		l.insert(nullptr, &delete_me[i]);
		size++;
	}
	ASSERT_EQ(l.size(), size);

	for (unsigned int i = 0 ; i < LIST_TESTSIZE ; ++i) {
		l.insert(&delete_me[i], &nodes[i]);
		size++;
	}
	ASSERT_EQ(l.size(), size);

	for (unsigned int i = 0 ; i < LIST_TESTSIZE ; ++i) {
		l.remove(&delete_me[i]);
		size--;
	}
	ASSERT_EQ(l.size(), size);

	unsigned int i = 0;
	for (auto & n : l) {
		ASSERT_EQ(n.data, i++);
	}

	l.clear();
	ASSERT_EQ(l.size(), 0);
	ASSERT_TRUE(l.empty());
}

#endif //YGG_TEST_LIST_HPP
