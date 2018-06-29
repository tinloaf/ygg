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

#define DYNSEGTREE_TESTSIZE 1500
#define DYNSEGTREE_DELETION_TESTSIZE 500
#define DYNSEGTREE_DELETION_ITERATIONS 100

// chosen by fair xkcd
#define DYNSEGTREE_SEED 4

namespace test_interval_agg {
using namespace boost::icl;
using namespace ygg;

using MCombiner = MaxCombiner<int, int>;
using Combiners = CombinerPack<int, int, MCombiner>;

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


using DynSegTree = DynamicSegmentTree<Node, NodeTraits, Combiners>;

TEST(DynSegTreeTest, TrivialTest)
{
	Node n(2,5,10);

	DynSegTree agg;
	ASSERT_TRUE(agg.empty());
	agg.insert(n);
	ASSERT_FALSE(agg.empty());

	int agg_val = agg.query(3);
	ASSERT_EQ(agg_val, 10);
	int combined = agg.get_combined<MCombiner>();
	ASSERT_EQ(combined, 10);

	int combined_range = agg.get_combined<MCombiner>(3,4);
	ASSERT_EQ(combined_range, 10);

	combined_range = agg.get_combined<MCombiner>(0,1);
	ASSERT_EQ(combined_range, 0);
	combined_range = agg.get_combined<MCombiner>(0,3);
	ASSERT_EQ(combined_range, 10);
	combined_range = agg.get_combined<MCombiner>(2,5);
	ASSERT_EQ(combined_range, 10);
	combined_range = agg.get_combined<MCombiner>(2,7);
	ASSERT_EQ(combined_range, 10);
	combined_range = agg.get_combined<MCombiner>(8,10);
	ASSERT_EQ(combined_range, 0);

	// Test iteration
	auto it = agg.cbegin();
	ASSERT_EQ(it->get_point(), 2);
	ASSERT_EQ(it->is_start(), true);
	ASSERT_EQ(static_cast<const Node *>(it->get_interval()), &n);
	it++;
	ASSERT_EQ(it->get_point(), 5);
	ASSERT_EQ(it->is_end(), true);
	ASSERT_EQ(static_cast<const Node *>(it->get_interval()), &n);
	it++;
	ASSERT_EQ(it, agg.cend());
}

TEST(DynSegTreeTest, TestEventBounding)
{
	Node n(2,5,10);

	DynSegTree agg;
	ASSERT_TRUE(agg.empty());
	agg.insert(n);
	ASSERT_FALSE(agg.empty());

	// Test finding the [2 border via upper and lower bounding
	auto it = agg.upper_bound_event(0);
	ASSERT_EQ(it, agg.begin());
	it = agg.lower_bound_event(0);
	ASSERT_EQ(it, agg.begin());

	// Test upper-bounding  vs lower bounding
	it = agg.lower_bound_event(2);
	ASSERT_EQ(it, agg.begin());
	it = agg.upper_bound_event(2);
	ASSERT_EQ(it, agg.begin() + 1);

	// Note that the 5) border is open - thus, lower-bouding on 5 actually correctly gives the
	// end iterator!
	it = agg.lower_bound_event(5);
	ASSERT_EQ(it, agg.end());
	it = agg.upper_bound_event(5);
	ASSERT_EQ(it, agg.end());
}

TEST(DynSegTreeTest, NestingTest)
{
	Node n[DYNSEGTREE_TESTSIZE];
	for (unsigned int i = 0 ; i < DYNSEGTREE_TESTSIZE ; ++i) {
		n[i].lower = i;
		n[i].upper = 2 * DYNSEGTREE_TESTSIZE - i + 1;
		n[i].value = 1;
	}

	DynSegTree agg;

	for (unsigned int i = 0 ; i < DYNSEGTREE_TESTSIZE ; ++i) {
		agg.insert(n[i]);
	}

	for (unsigned int i = 0 ; i < DYNSEGTREE_TESTSIZE ; ++i) {
		auto val = agg.query(i);
		ASSERT_EQ(val, i + 1);
	}

	int combined = agg.get_combined<MCombiner>();
	ASSERT_EQ(combined, DYNSEGTREE_TESTSIZE);

	for (unsigned int i = 0 ; i < DYNSEGTREE_TESTSIZE ; ++i) {
		int combined_range = agg.get_combined<MCombiner>(0,i+1,true,false);
		ASSERT_EQ(combined_range, i+1);
	}

	// Test iteration
	auto it = agg.cbegin();
	for (unsigned int i = 0 ; i < DYNSEGTREE_TESTSIZE ; ++i) {
		ASSERT_EQ(it->get_point(), i);
		ASSERT_TRUE(it->is_start());
		it++;
	}
	for (int i = DYNSEGTREE_TESTSIZE - 1;  i >= 0 ; --i) {
		ASSERT_EQ(it->get_point(), 2 * DYNSEGTREE_TESTSIZE - i + 1);
		ASSERT_TRUE(it->is_end());
		it++;
	}
	ASSERT_EQ(it, agg.cend());
}

TEST(DynSegTreeTest, OverlappingTest)
{
	Node n[DYNSEGTREE_TESTSIZE];
	for (unsigned int i = 0 ; i < DYNSEGTREE_TESTSIZE ; ++i) {
		n[i].lower = i;
		n[i].upper = DYNSEGTREE_TESTSIZE + i;
		n[i].value = 1;
	}

	DynSegTree agg;

	for (unsigned int i = 0 ; i < DYNSEGTREE_TESTSIZE ; ++i) {
		agg.insert(n[i]);
	}

	int combined = agg.get_combined<MCombiner>();
	ASSERT_EQ(combined, DYNSEGTREE_TESTSIZE);

	for (unsigned int i = 0 ; i < DYNSEGTREE_TESTSIZE ; ++i) {
		auto val = agg.query(i);
		ASSERT_EQ(val, i + 1);
	}
	for (unsigned int i = 0 ; i < DYNSEGTREE_TESTSIZE ; ++i) {
		auto val = agg.query(i + DYNSEGTREE_TESTSIZE);
		ASSERT_EQ(val, DYNSEGTREE_TESTSIZE - i - 1);
	}
}

TEST(DynSegTreeTest, DeletionTest)
{
	Node n[DYNSEGTREE_DELETION_TESTSIZE];
	for (unsigned int i = 0 ; i < DYNSEGTREE_DELETION_TESTSIZE ; ++i) {
		n[i].lower = i;
		n[i].upper = DYNSEGTREE_DELETION_TESTSIZE + i;
		n[i].value = 1;
	}

	DynSegTree agg;

	for (unsigned int i = 0 ; i < DYNSEGTREE_DELETION_TESTSIZE ; ++i) {
		agg.insert(n[i]);
	}

	std::mt19937 rng(DYNSEGTREE_SEED);

	for (unsigned int j = 0 ; j < DYNSEGTREE_DELETION_ITERATIONS ; ++j) {
		//std::cout << "\n\n\n=================================================\n\n\n";
		std::uniform_int_distribution<unsigned int> bounds_distr(0, 2 * DYNSEGTREE_DELETION_TESTSIZE);
		unsigned int lower = bounds_distr(rng);
		unsigned int upper = lower + bounds_distr(rng) + 1;

		Node deleteme(lower, upper, 42);
		//std::cout << "Test interval: " << lower << "->" << upper << "\n\n";

		//std::cout << "================= INSERT ====================\n";

		agg.insert(deleteme);

		for (unsigned int i = 0 ; i < DYNSEGTREE_DELETION_TESTSIZE ; ++i) {
			auto val = agg.query(i);
			if ((i >= lower) && (i < upper)) {
				ASSERT_EQ(val, i + 1 + 42);
			} else {
				ASSERT_EQ(val, i + 1);
			}
		}
		for (unsigned int i = 0 ; i < DYNSEGTREE_DELETION_TESTSIZE ; ++i) {
			auto val = agg.query(i + DYNSEGTREE_DELETION_TESTSIZE);
				if ((i  + DYNSEGTREE_DELETION_TESTSIZE >= lower) && (i  + DYNSEGTREE_DELETION_TESTSIZE < upper)) {
					ASSERT_EQ(val, DYNSEGTREE_DELETION_TESTSIZE - i - 1 + 42);
				} else {
					ASSERT_EQ(val, DYNSEGTREE_DELETION_TESTSIZE - i - 1);
				}
		}

		//std::cout << "================= REMOVE ====================\n";

		agg.remove(deleteme);

		for (unsigned int i = 0 ; i < DYNSEGTREE_DELETION_TESTSIZE ; ++i) {
			auto val = agg.query(i);
			ASSERT_EQ(val, i + 1);
		}
		for (unsigned int i = 0 ; i < DYNSEGTREE_DELETION_TESTSIZE ; ++i) {
			auto val = agg.query(i + DYNSEGTREE_DELETION_TESTSIZE);
			ASSERT_EQ(val, DYNSEGTREE_DELETION_TESTSIZE - i - 1);
		}

		int combined = agg.get_combined<MCombiner>();
		ASSERT_EQ(combined, DYNSEGTREE_DELETION_TESTSIZE);
	}
}

TEST(DynSegTreeTest, NestingTestInsertionOverlappingDeletionTest)
{
	Node n[DYNSEGTREE_TESTSIZE];
	for (unsigned int i = 0 ; i < DYNSEGTREE_TESTSIZE ; ++i) {
		n[i].lower = i;
		n[i].upper = 2 * DYNSEGTREE_TESTSIZE - i + 1;
		n[i].value = 1;
	}

	Node transient[DYNSEGTREE_TESTSIZE];
	for (unsigned int i = 0 ; i < DYNSEGTREE_TESTSIZE ; ++i) {
		transient[i].lower = i;
		transient[i].upper = DYNSEGTREE_TESTSIZE + i;
		transient[i].value = 10;
	}

	DynSegTree agg;

	for (unsigned int i = 0 ; i < DYNSEGTREE_TESTSIZE ; ++i) {
		agg.insert(n[i]);
		agg.insert(transient[DYNSEGTREE_TESTSIZE - i - 1]);
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

	for (unsigned int i = 2 ; i < DYNSEGTREE_TESTSIZE ; ++i) {
		agg.remove(transient[i]);
	}

	for (unsigned int i = 0 ; i < DYNSEGTREE_TESTSIZE ; ++i) {
		val = agg.query(i);
		ASSERT_EQ(val, i + 1);
	}
}

TEST(DynSegTreeTest, ManyEqualTest)
{
	Node n_middle[DYNSEGTREE_TESTSIZE];
	for (unsigned int i = 0 ; i < DYNSEGTREE_TESTSIZE ; ++i) {
		n_middle[i].lower = 10;
		n_middle[i].upper = 20;
		n_middle[i].value = 1;
	}

	Node n_left[DYNSEGTREE_TESTSIZE];
	for (unsigned int i = 0 ; i < DYNSEGTREE_TESTSIZE ; ++i) {
		n_left[i].lower = 0;
		n_left[i].upper = 15;
		n_left[i].value = 7;
	}

	Node n_right[DYNSEGTREE_TESTSIZE];
	for (unsigned int i = 0 ; i < DYNSEGTREE_TESTSIZE ; ++i) {
		n_right[i].lower = 15;
		n_right[i].upper = 25;
		n_right[i].value = 29;
	}


	DynSegTree agg;

	for (unsigned int i = 0 ; i < DYNSEGTREE_TESTSIZE ; ++i) {
		agg.insert(n_middle[i]);
		agg.insert(n_left[i]);
		agg.insert(n_right[i]);
	}

	auto val = agg.query(0);
	ASSERT_EQ(val, DYNSEGTREE_TESTSIZE * 7);
	val = agg.query(5);
	ASSERT_EQ(val, DYNSEGTREE_TESTSIZE * 7);
	val = agg.query(10);
	ASSERT_EQ(val, DYNSEGTREE_TESTSIZE * 7 + DYNSEGTREE_TESTSIZE * 1);
	val = agg.query(12);
	ASSERT_EQ(val, DYNSEGTREE_TESTSIZE * 7 + DYNSEGTREE_TESTSIZE * 1);
	val = agg.query(15);
	ASSERT_EQ(val, DYNSEGTREE_TESTSIZE * 1 + DYNSEGTREE_TESTSIZE * 29);
	val = agg.query(17);
	ASSERT_EQ(val, DYNSEGTREE_TESTSIZE * 1 + DYNSEGTREE_TESTSIZE * 29);
	val = agg.query(20);
	ASSERT_EQ(val, DYNSEGTREE_TESTSIZE * 29);
	val = agg.query(22);
	ASSERT_EQ(val, DYNSEGTREE_TESTSIZE * 29);
	val = agg.query(25);
	ASSERT_EQ(val, 0);
}

TEST(DynSegTreeTest, ComprehensiveTest)
{
	Node persistent_nodes[DYNSEGTREE_TESTSIZE];
	std::vector<unsigned int> indices;
	std::mt19937 rng(DYNSEGTREE_SEED);

	DynSegTree agg;

	for (unsigned int i = 0; i < DYNSEGTREE_TESTSIZE; ++i) {
		std::uniform_int_distribution<unsigned int> bounds_distr(0, 10 * DYNSEGTREE_TESTSIZE / 2);
		unsigned int lower = bounds_distr(rng);
		unsigned int upper = lower + 1 + bounds_distr(rng);

		persistent_nodes[i] = Node(lower, upper, i);
		indices.push_back(i);
	}

	Node transient_nodes[DYNSEGTREE_TESTSIZE];
	for (unsigned int i = 0; i < DYNSEGTREE_TESTSIZE; ++i) {
		std::uniform_int_distribution<unsigned int> bounds_distr(0, 10 * DYNSEGTREE_TESTSIZE / 2);
		unsigned int lower = bounds_distr(rng);
		unsigned int upper = lower + 1 + bounds_distr(rng);

		transient_nodes[i] = Node(lower, upper, DYNSEGTREE_TESTSIZE + i);
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

	int maxval = 0;

	auto it = reference.begin();
	while (it != reference.end()) {
		int lower = it->first.lower();
		int upper = it->first.upper();
		int val = it->second;

		for (int i = lower ; i < upper ; ++i) {
			int result = agg.query(i);
			ASSERT_EQ(val, result);
			maxval = std::max(result, maxval);
		}

		++it;
	}

	int combined = agg.get_combined<MCombiner>();
	ASSERT_EQ(combined, maxval);
}

TEST(DynSegTreeTest, ComprehensiveCombinerTest)
{
	std::mt19937 rng(DYNSEGTREE_SEED);

	Node persistent_nodes[DYNSEGTREE_TESTSIZE];
	std::vector<unsigned int> indices;

	DynSegTree agg;

	for (unsigned int i = 0; i < DYNSEGTREE_TESTSIZE; ++i) {
		std::uniform_int_distribution<unsigned int> bounds_distr(0, 10 * DYNSEGTREE_TESTSIZE / 2);
		unsigned int lower = bounds_distr(rng);
		unsigned int upper = lower + 1 + bounds_distr(rng);

		persistent_nodes[i] = Node(lower, upper, i);
		indices.push_back(i);
	}

	Node transient_nodes[DYNSEGTREE_TESTSIZE];
	for (unsigned int i = 0; i < DYNSEGTREE_TESTSIZE; ++i) {
		std::uniform_int_distribution<unsigned int> bounds_distr(0, 10 * DYNSEGTREE_TESTSIZE / 2);
		unsigned int lower = bounds_distr(rng);
		unsigned int upper = lower + 1 + bounds_distr(rng);

		transient_nodes[i] = Node(lower, upper, DYNSEGTREE_TESTSIZE + i);
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
		//std::cout << " Interval: [" << node.lower << ", " << node.upper << ") (@" << node.value
		//          << ")\n";
	}

	//std::cout << "Resulting Tree: \n";
	//std::cout << agg.dbg_get_dot().str();

	auto start_it = reference.begin();
	while (start_it != reference.end()) {
		auto end_it = start_it;
		++end_it;

		while (end_it != reference.end()) {
			auto it = start_it;
			int max_seen = it->second;
			int range_lower = start_it->first.lower();
			int range_upper = start_it->first.upper();

			bool upper_closed = false;
			while (it != end_it) {
				max_seen = std::max(max_seen, it->second);
				range_upper = it->first.upper();
				if (boost::icl::contains(it->first, it->first.upper())) {
					upper_closed = true;
				} else {
					upper_closed = false;
				}
				++it;
			}

			bool lower_closed = false;
			if (boost::icl::contains(start_it->first, start_it->first.lower())) {
				lower_closed = true;
			}

			int combined = agg.get_combined<MCombiner>(range_lower,
			                                           range_upper,
			                                           lower_closed,
			                                           upper_closed);
			/*
			std::cout << " -> Querying: ";
			if (lower_closed) {
				std::cout << " [";
			} else {
				std::cout << " (";
			}
			std::cout << range_lower << ", " << range_upper;
			if (upper_closed) {
				std::cout << "]";
			} else {
				std::cout << ")";
			}
			std::cout << "\n";
			*/
			ASSERT_EQ(combined, max_seen);

			++end_it;
		}
		++start_it;
	}
}

}

#endif //YGG_TEST_INTERVAL_AGGREGATOR_HPP
