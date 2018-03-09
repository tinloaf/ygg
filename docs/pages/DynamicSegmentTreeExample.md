@page dynamicsegmenttreeexample DynamicSegmentTree Example

This page provides an example of how to use the dynamic segment tree. For information on what a
 dynamic segment tree is and how it behaves, please see @ref dynamicsegmenttreedoc.

The code can also be found in the examples directory.

~~~~~~~~~~~~~{.cpp}
#include "../src/ygg.hpp"

using namespace ygg;

/* First, we define which combiners we want to use. For this simple example, we use a
 * simple provided MaxCombiner. See TODO for other combiners.
 */
using MCombiner = MaxCombiner<int>;
using Combiners = CombinerPack<int, MCombiner>;

/* The class representing our intervals and associated values.
 *
 * Our key space (i.e., the borders of the intervals) will be double.
 *
 * The while the values will be ints. The DynamicSegmentTree aggregates values by using
 * operator+, thus our aggregation will be simply the sum of the aggregated values. We need to
 * specify the type of the aggregate values, too.
 *
 *
 */
class Interval : public DynSegTreeNodeBase<double, int, int, Combiners>  {
public:
	double lower;   // lower interval border
	double upper;   // upper interval border

	int value;      // value associated with this interval.

	Interval(double lower_in, double upper_in, int value_in) : lower(lower_in), upper(upper_in),
	                                                           value(value_in) {};
};

/* We need to define traits on our interval class, derived from DynSegTreeNodeTraits<Interval>.
 * This class tells the DynamicSegmentTree how to get the interval borders and the value
 * from our Interval class.
 */
class IntervalTraits : public DynSegTreeNodeTraits<Interval> {
public:
	using key_type = double;
	using value_type = int;

	static key_type get_lower(const Interval & n) {
		return n.lower;
	}

	static key_type get_upper(const Interval & n) {
		return n.upper;
	}

	static value_type get_value(const Interval & n) {
		return n.value;
	}

	/* All our intervals are right-open. The DynamicSegmentTree can handle every combination
	 * of openness / closedness, and in fact can contain intervals of different types.
	 */
	static bool is_lower_closed(const Interval & n) {
		(void)n;
		return true;    // closed on the lower border
	};
	static bool is_upper_closed(const Interval & n) {
		(void)n;
		return false;   // open on the upper border
	};
};

/* This is our DynamicSegmentTree
 */
using MyTree = DynamicSegmentTree<Interval, IntervalTraits, Combiners>;


int main(int argc, char **argv) {
	(void)argc;
	(void)argv;

	MyTree t;

	// Storage for the actual intervals.
	// WARNING: using STL containers here can backfire badly. See TODO.
	std::vector<Interval> intervals;

	/* We add the following interval / value combinations:
	 *
	 *  [0,   10)   -> 1
	 *  [0.5, 10)   -> 2
	 *  [10,  15)   -> 3
	 *  [12,  20)   -> 8
	 */
	intervals.emplace_back(0.0, 10.0, 1);
	intervals.emplace_back(0.5, 10.0, 2);
	intervals.emplace_back(10.0, 15.0, 3);
	intervals.emplace_back(12.0, 20.0, 8);
	for (auto & interval : intervals) {
		t.insert(interval);
	}

	/* Let's see what the aggregate value at the points 0, 0.5, 5, 10 and 15 is: */
	for (auto point : std::vector<double>{0, 0.5, 5, 10, 14, 15}) {
		std::cout << "Point: " << point << "\t| Aggregate Value: " << t.query(point) << "\n";
	}

	/* Should output:
	 * Point: 0	  | Aggregate Value: 1
	 * Point: 0.5	| Aggregate Value: 3
	 * Point: 5	  | Aggregate Value: 3
	 * Point: 10	| Aggregate Value: 3
	 * Point: 15	| Aggregate Value: 8
	 * */

	/* Now, combiners. The max combiner first allows us to query (in O(1)) what the total maximum
	 * value is: */
	std::cout << "Maximum Value: " << t.get_combined<MCombiner>() << "\n";
	/* Should output:
	 * Maximum Value: 11
	 */

	/* But, we can also query (in O(log n)) the maximum over sub-ranges:*/
	std::cout << "Maximum over [0, 10): " << t.get_combined<MCombiner>(0, 10) << "\n";
	std::cout << "Maximum over [10, 12): " << t.get_combined<MCombiner>(10, 12) << "\n";
	// We also can specify to query an interval closed on both sides:
	std::cout << "Maximum over [10, 12]: " << t.get_combined<MCombiner>(10, 12, true, true) << "\n";

	/* Overall output should be:
	 * Maximum over [0, 10): 3
	 * Maximum over [10, 12): 3
	 * Maximum over [10, 12]: 11
	 */

	return 0;
}
~~~~~~~~~~~~~