@page datastructuresoverview Overview over the Data Structures

This page roughly explains what a Red-Black-Tree, an Interval Tree, an Interval Map or 
a Dynamic Segment Tree can do for you.
 
Red-Black-Tree
==============

A red-black tree is a search tree. It allows you to insert elements (nodes) into it, which are 
associated with some key. Inside the red-black tree, the elements will be held sorted according 
to their key. This makes two operations very efficient: Iterating all keys in sorted order, and 
finding a certain element by key.

For an example on how to use the red-black tree, see @ref rbtreeexample .

Zip Tree
========

A zip tree is a search tree, just as the red-black tree. It is a randomized data structure
that has some nice expected guarantees. For details, refer to
[the article by Tarjan et al.](https://arxiv.org/abs/1806.06726).

For an example on how to use the zip tree, see @ref ziptreeexample .

Interval Tree
=============

An interval tree is an extension of (in this case) a red-black tree. It allows you to insert 
elements (nodes) into it which are associated with an interval instead of a key. It stores the intervals sorted by
their lower interval border plus some meta information. This makes two operations very 
efficient: Iterating all intervals by their lower border, and finding (iterating) all intervals 
that overlap with a given query interval.
 
For an example on how to use the interval tree, see @ref intervaltreeexample .

Interval Map
============

An interval map stores intervals associated with a certain value. It is somewhat like an interval 
tree, but aggregates the intervals' values on overlap. For example, let's say you insert the 
interval [0, 10) with value 1, the interval [5, 15) with value 10 and the interval [15, 20) also 
with value 10. The interval map will now contain three *Segments*: 

* A segment from 0 to 5 having the (aggregated) value 1 (only the first 
interval participates in this segment)
* A segment from 5 to 10 having the aggregated value 11 (both the first and the second interval
participate here)
* A segment from 10 to 20 having the aggregated value 10. Note that this segment spans two 
intervals (the second and third), but since they have the same value, there is no segment border 
at 15.

For an example on how to use the interval map, see @ref intervalmapexample .

Dynamic Segment Tree
====================

The dynamic segment tree is something between a classic segment tree and Boost ICL's 
[interval_map](http://www.boost.org/doc/libs/1_66_0/libs/icl/doc/html/index.html#boost_icl.introduction.definition_and_basic_example). 
You can find a more detailed description at @ref dynamicsegmenttreedoc and an example of its usage 
at @ref dynamicsegmenttreeexample.