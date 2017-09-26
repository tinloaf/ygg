@page datastructuresoverview Overview over the Data Structures

This page roughly explains what a Red-Black-Tree, an Interval Tree and an Interval Map can do for
 you.
 
Red-Black-Tree
==============

A red-black tree is a search tree. It allows you to insert elements (nodes) into it, which are 
associated with some key. Inside the red-black tree, the elements will be held sorted according 
to their key. This makes two operations very efficient: Iterating all keys in sorted order, and 
finding a certain element by key.

For an example on how to use the red-black tree, see @ref rbtreexample .

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