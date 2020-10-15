@page datastructuresoverview Overview over the Data Structures

This page roughly explains what a Red-Black-Tree, an Interval Tree, an Interval Map or 
a Dynamic Segment Tree can do for you.
 
Red-Black-Tree
==============

A red-black tree is a balanced binary search tree. It allows you to insert elements (nodes) into it, which are 
associated with some key. Inside the red-black tree, the elements will be held sorted according 
to their key. This makes two operations very efficient: Iterating all keys in sorted order, and 
finding a certain element by key.

For an example on how to use the red-black tree, see @ref rbtreeexample .

Zip Tree
========

A zip tree is a balanced binary search tree, just as the red-black tree. It is a randomized data structure
that has some nice expected guarantees. For details, refer to
[the article by Tarjan et al.](https://arxiv.org/abs/1806.06726).

For an example on how to use the zip tree, see @ref ziptreeexample .

Weight Balanced Tree
========

A weight balanced tree (also known as BB[α]-tree) is a balanced binary search tree. It balances subtrees
based on the number of nodes in the respective subtrees.

For an example on how to use the weight balanced tree, see @ref wbtreeexample .


Interval Tree
=============

An interval tree is an extension of (in this case) a red-black tree. It allows you to insert 
elements (nodes) into it which are associated with an interval instead of a key. It stores the intervals sorted by
their lower interval border plus some meta information. This makes two operations very 
efficient: Iterating all intervals by their lower border, and finding (iterating) all intervals 
that overlap with a given query interval.
 
For an example on how to use the interval tree, see @ref intervaltreeexample .

Dynamic Segment Tree
====================

The dynamic segment tree is something between a classic segment tree and Boost ICL's 
[interval_map](http://www.boost.org/doc/libs/1_66_0/libs/icl/doc/html/index.html#boost_icl.introduction.definition_and_basic_example). It needs an underlying balanced binary search tree, for which you can either use the supplied red-black tree, the weight balanced tree or the Zip Tree.
You can find a more detailed description at @ref dynamicsegmenttreedoc and an example of its usage 
at @ref dynamicsegmenttreeexample.
