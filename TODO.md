* 'default comparable?'
* profiling
* end()--? (* mark end iterator differently)
* lower_bound
* test upper_bound / interval_upper_bound
* fine-tune link fixup at swap and rotation
* Interval Tree: Reduce the number of fixup operations
* make early fails configurable
* kill move-constructor & copy-constructor & copy-assignment
* make helper functions use SFINAE
* move node base into tree
* When we pass NB around, we probably don't need NodeTraits anymore?
* Multi-Trees: add more tests
* Add layer on top of IntervalMap segments to access borders etc.
* rename internal-namespaces to respective classes
* add tags to inner nodes in interval map
* rename ITree -> Tree in interval map
* rename Segment -> InnerNode in interval map
* replace accessing _rbt_* in IntervalTree
* Add combiner for collecting intervals