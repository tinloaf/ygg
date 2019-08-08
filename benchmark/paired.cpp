#include "paired.hpp"

#include "common.hpp"
#include "common_bst.hpp"

#include <fstream>

struct BSTInsertOptions : public DefaultBenchmarkOptions
{
	using MainRandomizer = UseUniform;
	constexpr static bool need_nodes = true;
	using NodeRandomizer = UseUniform;
#ifdef PRESORT
	constexpr static bool nodes_presort = true;
	constexpr static double nodes_presort_fraction = 0.5;
	constexpr static bool fixed_presort = true;
	constexpr static double fixed_presort_fraction = 0.5;
#endif
};

struct BSTEraseOptions : public DefaultBenchmarkOptions
{
	using MainRandomizer = UseUniform;
	constexpr static bool need_node_pointers = true;
	using NodePointerRandomizer = UseUniform;

	constexpr static bool distinct = true;

#ifdef PRESORT
	constexpr static bool fixed_presort = true;
	constexpr static double fixed_presort_fraction = 0.5;
	constexpr static bool pointers_presort = true;
	constexpr static double pointers_presort_fraction = 0.5;
#endif
};

struct BSTMoveOptions : public DefaultBenchmarkOptions
{
	using MainRandomizer = UseUniform;

	constexpr static bool need_values = true;
	using ValueRandomizer = UseUniform;
	constexpr static size_t node_value_change_percentage = 5;
	constexpr static bool need_node_pointers = true;
	using NodePointerRandomizer = DYN_GENERATOR;

#ifdef PRESORT
	constexpr static bool nodes_presort = true;
	constexpr static double nodes_presort_fraction = 0.5;
	constexpr static bool pointers_presort = true;
	constexpr static double pointers_presort_fraction = 0.5;
#endif
};

struct MoveExecutor
{
	template <class Fixture>
	static void
	run(Fixture & f)
	{
		for (size_t i = 0; i < f.experiment_node_pointers.size(); i++) {
			auto * n = f.experiment_node_pointers[i];
			auto new_val = f.experiment_values[i];

			f.t.remove(*n);
			n->set_value(new_val);
			f.t.insert(*n);
		}
	}

	template <class Fixture>
	static void
	revert(Fixture & f)
	{
		for (size_t i = 0; i < f.experiment_node_pointers.size(); i++) {
			auto * n = f.experiment_node_pointers[i];
			auto old_val = f.fixed_values[i];

			f.t.remove(*n);
			n->set_value(old_val);
			f.t.insert(*n);
		}
	}
};

struct InsertExecutor
{
	template <class Fixture>
	static void
	run(Fixture & f)
	{
		for (auto & n : f.experiment_nodes) {
			f.t.insert(n);
		}
	}

	template <class Fixture>
	static void
	revert(Fixture & f)
	{
		for (auto & n : f.experiment_nodes) {
			f.t.remove(n);
		}
	}
};

struct EraseExecutor
{
	template <class Fixture>
	static void
	run(Fixture & f)
	{
		for (auto n : f.experiment_node_pointers) {
			f.t.erase(n->get_value());
		}
	}

	template <class Fixture>
	static void
	revert(Fixture & f)
	{
		for (auto & n : f.experiment_node_pointers) {
			f.t.insert(*n);
		}
	}
};

template <class Executor, class Options, class Experiment, class OuterInterface,
          class... InnerInterfaces>
void
run_inner(const char * name, std::ofstream & out)
{
	std::vector<std::pair<Result, std::string>> res;

	(res.push_back(
	     {PairedTester<OuterInterface, InnerInterfaces, Experiment, Options>()
	          .template run<Executor>(100, 100, 2000000, 2000, 42),
	      InnerInterfaces::get_name()}),
	 ...);

	/*
 PairedTester<OuterInterface, InnerInterfaces, Experiment, Options> pt ,
 auto result = pt.template run<Executor>(100, 100, 2000000, 2000, 42) ,
	*/

	for (const auto & result : res) {
		out << name << "," << OuterInterface::get_name() << "," << result.second
		    << "," << result.first.p_value << "," << result.first.cohens_d << "\n";
	}
}

template <class Executor, class Options, class Experiment, class... Interfaces>
struct run_all;

template <class Executor, class Options, class Experiment, class First,
          class... Rest>
struct run_all<Executor, Options, Experiment, First, Rest...>
{
	void
	operator()(const char * name, std::ofstream & out)
	{
		run_inner<Executor, Options, Experiment, First, Rest...>(name, out);
		run_all<Executor, Options, Experiment, Rest...>{}(name, out);
	}
};

template <class Executor, class Options, class Experiment>
struct run_all<Executor, Options, Experiment>
{
	void
	operator()(const char * name, std::ofstream & out)
	{
		(void)name;
		(void)out;
	}
};

int
main(int argc, const char ** argv)
{
	assert(argc == 2);
	std::ofstream out(argv[1]);

	run_all<InsertExecutor, BSTInsertOptions, InsertExperiment,
	        YggRBTreeInterface<BasicTreeOptions>,
	        YggWBTreeInterface<WBTTwopassTreeOptions, WBBSTNamerDefGDefDTP>,
	        YggWBTreeInterface<WBTSinglepassTreeOptions, WBBSTNamerDefGDefDSP>,
	        YggWBTreeInterface<WBTSinglepassLWTreeOptions, WBBSTNamerLWSP>,
	        YggWBTreeInterface<WBTSinglepassBalTreeOptions, WBBSTNamerBalSP>,
	        YggWBTreeInterface<WBTSinglepassSuperBalTreeOptions,
	                           WBBSTNamerSuperBalSP>,
	        YggWBTreeInterface<WBTSinglepass32TreeOptions, WBBSTNamer3G2DSP>

	        >{}("Insert", out);

	run_all<EraseExecutor, BSTEraseOptions, EraseExperiment,
	        YggRBTreeInterface<BasicTreeOptions>,
	        YggWBTreeInterface<WBTTwopassTreeOptions, WBBSTNamerDefGDefDTP>,
	        YggWBTreeInterface<WBTSinglepassTreeOptions, WBBSTNamerDefGDefDSP>,
	        YggWBTreeInterface<WBTSinglepassLWTreeOptions, WBBSTNamerLWSP>,
	        YggWBTreeInterface<WBTSinglepassBalTreeOptions, WBBSTNamerBalSP>,
	        YggWBTreeInterface<WBTSinglepassSuperBalTreeOptions,
	                           WBBSTNamerSuperBalSP>,
	        YggWBTreeInterface<WBTSinglepass32TreeOptions, WBBSTNamer3G2DSP>

	        >{}("Erase", out);
	run_all<MoveExecutor, BSTMoveOptions, MoveExperiment,
	        YggRBTreeInterface<BasicTreeOptions>,
	        YggWBTreeInterface<WBTTwopassTreeOptions, WBBSTNamerDefGDefDTP>,
	        YggWBTreeInterface<WBTSinglepassTreeOptions, WBBSTNamerDefGDefDSP>,
	        YggWBTreeInterface<WBTSinglepassLWTreeOptions, WBBSTNamerLWSP>,
	        YggWBTreeInterface<WBTSinglepassBalTreeOptions, WBBSTNamerBalSP>,
	        YggWBTreeInterface<WBTSinglepassSuperBalTreeOptions,
	                           WBBSTNamerSuperBalSP>,
	        YggWBTreeInterface<WBTSinglepass32TreeOptions, WBBSTNamer3G2DSP>

	        >{}("Move", out);
}
