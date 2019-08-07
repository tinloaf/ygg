#include "paired.hpp"

#include "common.hpp"
#include "common_bst.hpp"

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

int
main()
{
	PairedTester<YggWBTreeInterface<WBTTwopassTreeOptions, WBBSTNamer3G2DTP>,
	             YggWBTreeInterface<WBTSinglepassTreeOptions, WBBSTNamer3G2DSP>,
	             InsertExperiment, BSTInsertOptions>
	    pt;
	auto result = pt.run<InsertExecutor>(100, 100, 2000000, 2000, 42);
	std::cout << "P: " << result.p_value << "\n";
	std::cout << "Effect Size: " << result.cohens_d << "\n";
}
