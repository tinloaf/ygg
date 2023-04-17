This file describes how to run benchmarks on (some of) the provided data structures. All benchmarking code
is supplied in the 'benchmark' subfolder. 

Building
========

To build the benchmarking tools, you will need:

* CMake >= 3.8
* git
* A C++ compiler with full C++17 support (GCC 7 and Clang 5 should do)
* If you want to take PAPI measurements: LibPAPI

Assuming that you unpacked Ygg to /path/to/ygg, building should be as easy as:

```
cd /path/to/ygg
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

Note that if you don't use the `-DCMAKE_BUILD_TYPE` switch, optimization is turned off and you will see a warning if you run the benchmarks.

Running Benchmarks
==================

After building, there are several types of benchmarks available in the folder `/path/to/ygg/build/benchmark`. The most basic benchmarks measure the time it takes to perform basic operations on the data structures. Other benchmarks analyze e.g. tree balance.

Timing Benchmarks
-----------------
The benchmarks to analyze the execution time of operations on the various binary search trees all start with `bench_bst_`. Equivalently, the binaries starting with `bench_dst_` benchmark operations on the dynamic segment tree. If the binary ends in `_papi`, it collects PAPI statistics instead of timing (see below). 

There are four different ways the keys for the benchmarked trees can be generated:
	* A uniform distribution. This is the default.
	* A Zipf distribution. This is the case in the binaries ending in `_zipf`.
	* A skewed distribution as suggested by Mäkinen [1]. This is the case in the binaries ending in `_skewed`.
	* A partially presorted set of keys. This is the case in the binaries ending in `_presorted`.

The timing benchmarks output lines of this form:

```
--------------------------------------------------------------------------------------------------------
Benchmark                                                              Time             CPU   Iterations
--------------------------------------------------------------------------------------------------------
BST :: Insert :: RBTree /2048/1000/4                              336143 ns       336180 ns         2116
```

The benchmark name contains the following information:
	* Whether it is a binary search tree (BST) or dynamic segment tree (DST) benchmark
	* The operation being benchmarked (the 'insert' operation, in this case)
	* The tree that is being benchmarked (the red-black tree, in this case)
	* The size of the initial tree being generated (2048 nodes in this case)
	* The number of times the benchmarked operation (insert) is executed on that tree (1000 in this case)
	* The random seed used (4)
	
The two other columns list the wall time elapsed, the CPU time elapsed, and the number of times that the Google Benchmark framework executed the benchmark.

Several parameters can be controlled via command-line switches. The binaries accept all parameters usually acceptey by Google Benchmark [2] - see the respective documentation especially on how to control output format. Additional command-line parameters are:

	* --base_size: The smallest size of the initially generated tree
	* --doublings: The number of times the base_size is doubled. 
	* --seed_start: The lowest value for the random seed
	* --seed_count: The number of seeds that should be tried
	* --experiment_size: The number of times that the operation to be benchmarked should be executed on the initially generated tree. Note that for some operations (delete, erase), it does not make sense to set experiment_size larger than base_size.
	* --filter: Allows to specify a regular expression on the 'benchmark name' field to filter benchmarks.
	
So, for example, running `--base_size 1024 --doublings 3 --seed_start 42 --seed_count 2` would execute each benchmark eight times: For tree sizes of 1024, 2048, 4096 and 8192, each with the seeds 42 and 43.


Troubleshooting
===============

Message '***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.'
----------------------------------------------

CPU scaling interferes with time measurements. For best results, please run

`cpufreq-set -g performance -c <Core-Nr>`

for each of your CPU-Cores.


References
==========
[1] Erkki Mäkinen. On top-down splaying. BIT Numerical Mathematics, 27(3):330–339, Sep 1987. doi:10.1007/BF01933728.
[2] https://github.com/google/benchmark
