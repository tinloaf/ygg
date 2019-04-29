@page dynamicsegmenttreedoc Introduction to the Dynamic Segment Tree

The dynamic segment tree is something between a classic segment tree and Boost ICL's 
[interval_map](http://www.boost.org/doc/libs/1_66_0/libs/icl/doc/html/index.html#boost_icl.introduction.definition_and_basic_example). 
You can find an example of its usage at @ref dynamicsegmenttreeexample.

It allows you to insert (and remove) intervals that are associated with a *value*. For every point 
**x**
in the key space, you can query for the *aggregate* of the values that contain **x**. You are 
free to define what an aggregate is. If your values are numbers, a reasonable aggregate may be 
the sum. If your values are lists of objects, an aggregate may be the concatenation of the lists, …

This is more or less what Boost ICL's *interval_map* offers. On top of that, you can define (or 
use the pre-defined) *combiners*. Combiners combine different aggregate values over an 
interval in the key space. For example, if your values are numbers, the "max" function is a 
reasonable (and in fact, pre-defined) combiner. Adding this combiner to your dynamic segment tree
 allows you to efficiently (see TODO) query for the maximum aggregated value over any arbitrary 
 interval in your key space. Possible combiners need to satisfy a couple of constraints, see TODO.

The DynamicSegmentTree needs an underlying balanced binary search tree, for which you can either use the supplied red-black tree, 
the weight balanced tree or the Zip Tree. The selection is done via the TreeSelector template parameter.

An in-depth technical description of how the dynamic segment tree works can be found at TODO.

Requirements
------------

* If you use `double` as key type, interval borders will be compared using `operator==` and 
`operator!=`. Thus, if you want two intervals to begin / end at the same point, you must make 
sure that their borders really contain the same `double`.
* The aggregate type must offer a neutral element, which also must be the default-constructed value 
of the type.
* On the aggregate and value type, `operator+` and `operator-` must be defined. Together with the
 requirement that you need a neutral element, this especially means that you need to have a 
 "negative aggregate space": If `i` is the neutral element, and `x` is any element of your value 
 / aggregate type, then `i - x` must also be valid in your aggregate type!
