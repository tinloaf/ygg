@mainpage

This is Ygg (short for [Yggdrasil](https://en.wikipedia.org/wiki/Yggdrasil)), a C++17
implementation of intrusive search tree and interval data structures. It currently contains:

* several balanced binary search trees:
  * a red-black tree
  * a weight balanced tree (aka BB[α]-tree)
  * a zip tree
* an interval tree
* an interval map
* a dynamic segment tree
  * based on a red-black tree
  * based on a zip tree
* a doubly-linked list


If you need a Red-Black-Tree, a Zip Tree, an Interval Tree or an Interval Map in your C++ application, and for 
some reason the existing implementations (like std::set or boost::instrusive::rbtree) are not 
suited for you, Ygg may be the answer. Also, I do not know of any other implementation of the 
"Dynamic Segment Tree" (if you know something similar, please let me know!)

See the list of features below for why Ygg is awesome!

If you are not sure whether one of these data structures is right for your application, check out
 the @ref datastructuresoverview.

Features
========

* All the data structures are intrusive! Like the containers in boost::intrusive, Ygg follows a 
'bring your own data structure' approach. Depending on your use case, this can save a lot of time by avoiding memory allocation and copying stuff around.
* You can be notified when stuff happens. Want to do something to the tree nodes every time a 
tree rotation happens? Need to know whenever two nodes swap places in the tree? Look no further. Ygg will call methods specified by you on several occasions. In fact, that's how the Interval Tree (the augmented tree version from Cormen et al.) is implemented.
* It's pretty fast. Doing benchmarks correctly is pretty difficult. However, performance should 
not be too far away from boost::intrusive::rbtree. Comparing against std::set is unfair - std::set loses because it needs to do memory allocation.

Installation
============

It's a header-only library. (Yes I know, there are .cpp files. I like to keep declaration and 
definition separated, even if everything's actually a header.) Just make sure everything in the 
src folder is in your include path, and you're set.

Getting Started
=============

If you already know which data structure you want to use (if not, check out @ref 
datastructuresoverview), the examples (which can also be found in the examples folder) might be 
the easiest way to get started:

* Example of using the red-black tree: @ref rbtreeexample
* Example of using the zip tree: @ref ziptreeexample
* Example of using the weight balanced tree: @ref wbtreeexample
* Example of using the interval tree: @ref intervaltreeexample
* Example of using the interval map: @ref intervalmapexample
* Example of using the dynamic segment tree: @ref dynamicsegmenttreeexample

There's also a @ref quickstart page that walks you through the examples for the red-black tree 
and the interval tree.

Documentation
=============

The API is somewhat documented. Check out the menu above for documentation on the individual 
classes.

License
=======

This software is licensed under the MIT license. See @ref license for details.
