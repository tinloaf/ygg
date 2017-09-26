@page intervalmapexample IntervalMap Example

This page provides an example of how to use the interval map. For information on what an interval
 map is and how it behaves, please see @ref datastructuresoverview.

The code can also be found in the examples directory.

~~~~~~~~~~~~~{.cpp}
#include "ygg.hpp"

#include <vector>

using namespace ygg;

/* The node class. It provides a simple interval -> value mapping, where interval borders as well
 * as the value associated with an interval are integers.
 */
class Node : public IMapNodeBase<int, int> {
public:
	int lower;
	int upper;
	int value;

	Node(int lower_in, int upper_in, int value_in) : lower(lower_in), upper(upper_in), value(value_in)
	{}
};

/* We need to specify our own NodeTraits. This class is used by the IntervalMap to retrieve the
 * interval borders as well as the value from the nodes.
 */
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

/* Configure the interval map with the node and traits class */
using IMap = IntervalMap<Node, NodeTraits>;

int main(int argc, char **argv) {
	(void)argc;
	(void)argv;

	/* Create some intervals:
	 * [0,1):   42
	 * [2,3):   23
	 * [3,10):  23
	 * [5,20):  47
	 *
	 * When all of them are inserted into an IntervalMap, this should result in:
	 * [0,1):   42
	 * [1,2):   0
	 * [2,5):   23
	 * [5,10):  70
	 * [10,20): 47
	 */
	std::vector<Node> nodes;
	nodes.emplace_back(0, 1, 42);
	nodes.emplace_back(2, 3, 23);
	nodes.emplace_back(3, 10, 23);
	nodes.emplace_back(5, 20, 47);

	IMap m;

	/* Insert all the nodes into the interval map */
	for (auto & n : nodes) {
		m.insert(n);
	}

	/* Iterate and output all segments. */
	for (auto it = m.begin(); it != m.end(); ++it) {
		std::cout << "Segment from " << it.get_lower() << " to " << it.get_upper() << ": aggregate "
						"value " << it.get_value() << "\n";
	}

	/* This should output:
	 *
	 * Segment from 0 to 1: aggregate value 42
	 * Segment from 1 to 2: aggregate value 0
	 * Segment from 2 to 5: aggregate value 23
	 * Segment from 5 to 10: aggregate value 70
	 * Segment from 10 to 20: aggregate value 47
	 */

	return 0;
}


~~~~~~~~~~~~~