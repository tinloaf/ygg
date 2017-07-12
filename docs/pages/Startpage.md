@mainpage

This is Ygg (short for [Yggdrasil](https://en.wikipedia.org/wiki/Yggdrasil)), a C++11 implementation of a Red-Black-Tree as well as an Interval Tree based on the Red-Black-Tree.

If you need a Red-Black- or Interval Tree in your C++ application, and for some reason the existing implementations (like std::set or boost::instrusive::rbtree) are not suited for you, Ygg may be the answer. See the list of features below for why it's awesome!

Features
========

* It's intrusive! Like the containers in boost::intrusive, Ygg follows a 'bring your own data structure' approach. Depending on your use case, this can save a lot of time by avoiding memory allocation and copying stuff around.
* You can be notified when stuff happens. Want to do something to the tree nodes every time a tree rotation happens? Need to know whenever two nodes swap places in the tree? Look no further. Ygg will call methods specified by you on several occasions. In fact, that's how the Interval Tree (the augmented tree version from Cormen et al.) is implemented.
* It's pretty fast. Doing benchmarks correctly is pretty difficult. However, performance should not be too far away from boost::intrusive::rbtree. Comparing against std::set is unfair - std::set loses because it needs to do memory allocation.

Installation
============

It's a header-only library. (Yes I know, there are .cpp files. I like to keep declaration and definition separated, even if everything's actually a header.) Just make sure everything in the src folder is in your include path, and you're set.

Documentation
=============

There's a @ref quickstart page that should get you up and running pretty quickly, and some class documentation (see the menu above).


License
=======

This software is licensed under the MIT license. See @ref license for details.
