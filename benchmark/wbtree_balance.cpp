#include "../src/options.hpp"
#include "../src/wbtree.hpp"
#include <algorithm>
#include <random>

using namespace ygg;

template <class InitialFlags, class Flags>
class BalanceAnalyzer {
private:
	constexpr static size_t COUNT_INTERVAL = 1000;

	template <class NodeFlags>
	class Node : public weight::WBTreeNodeBase<Node<NodeFlags>, NodeFlags> {
	public:
		int key;

		bool
		operator<(const Node & other) const
		{
			return this->key < other.key;
		}
	};

	using InitialTree = weight::WBTree<Node<InitialFlags>,
	                                   weight::WBDefaultNodeTraits, InitialFlags>;
	using Tree = weight::WBTree<Node<Flags>, weight::WBDefaultNodeTraits, Flags>;

public:
	BalanceAnalyzer(size_t node_count_in, size_t reinsertion_count_in,
	                std::ofstream & series_of_in, std::ofstream & distr_of_in)
	    : node_count(node_count_in), reinsertion_count(reinsertion_count_in),
	      series_of(series_of_in), distr_of(distr_of_in)
	{
		this->nodes.resize(this->node_count);
	}

	void
	run(int seed)
	{
		std::mt19937 rnd(seed);
		std::uniform_int_distribution distr(std::numeric_limits<int>::min(),
		                                    std::numeric_limits<int>::max());

		this->init_t.clear();

		for (size_t i = 0; i < this->node_count; ++i) {
			this->nodes[i].key = distr(rnd);
			this->init_t.insert(*(Node<InitialFlags> *)(&this->nodes[i]));
		}

		// Bad Hack.
		this->t = (Tree *)(&this->init_t);

		for (size_t i = 0; i < this->reinsertion_count; ++i) {
			size_t index = std::abs(distr(rnd)) % this->node_count;

			// TODO optimistic - also needs distinct values!
			this->init_t.remove(*(Node<InitialFlags> *)(&this->nodes[index]));
			//			this->t->remove(this->nodes[index]);
			//			assert(this->t->dbg_count_violations() == 0);
			this->nodes[index].key = distr(rnd);
			this->t->insert(this->nodes[index]);

			if (i % COUNT_INTERVAL == 0) {
				this->series_of << seed << "," << i << ","
				                << this->t->dbg_count_violations() << "\n";
			}
		}

		std::vector<size_t> depths;
		this->counts.push_back(this->t->dbg_count_violations(&depths));

		for (size_t depth = 0; depth < depths.size(); ++depth) {
			this->distr_of << seed << "," << depth << "," << depths[depth] << "\n";
		}
		this->distr_of << std::flush;
		this->series_of << std::flush;
	}

	void
	print()
	{
		std::cout << "Max: \t"
		          << *(std::max_element(this->counts.begin(), this->counts.end()))
		          << "\n";
		std::cout << "Min: \t"
		          << *(std::min_element(this->counts.begin(), this->counts.end()))
		          << "\n";
		size_t sum = std::accumulate(this->counts.begin(), this->counts.end(), 0);
		std::cout << "Mean: \t" << (double)sum / (double)this->counts.size()
		          << "\n";
		if (this->counts.size() % 2 == 0) {
			std::nth_element(this->counts.begin(),
			                 this->counts.begin() + (this->counts.size() / 2),
			                 this->counts.end());
			std::nth_element(this->counts.begin(),
			                 this->counts.begin() + (this->counts.size() / 2) - 1,
			                 this->counts.end());
			std::cout << "Median: \t"
			          << (double)(this->counts[this->counts.size() / 2] +
			                      this->counts[this->counts.size() / 2 - 1]) /
			                 2.0
			          << "\n";
		} else {
			std::nth_element(this->counts.begin(),
			                 this->counts.begin() + (size_t)(this->counts.size() / 2),
			                 this->counts.end());
			std::cout << "Median: \t"
			          << this->counts[(size_t)(this->counts.size() / 2)] << "\n";
		}
	}

private:
	size_t node_count;
	size_t reinsertion_count;
	std::vector<Node<Flags>> nodes;
	std::vector<size_t> counts;
	InitialTree init_t;
	Tree * t;
	std::ofstream & series_of;
	std::ofstream & distr_of;
};

int
main(int argc, char ** argv)
{
	size_t node_count = atoi(argv[1]);
	size_t reinsertion_count = atoi(argv[2]);
	size_t seed_count = atoi(argv[3]);

	// CSV file to write gradual counts to
	std::string out_dir(argv[4]);
	std::string series_fname = out_dir + "/series.csv";
	std::ofstream series_os(series_fname, std::ios::trunc);
	series_os << "Seed,Iteration,Count\n";

	std::string distr_fname = out_dir + "/distribution.csv";
	std::ofstream distr_os(distr_fname, std::ios::trunc);
	distr_os << "Seed,Depth,Count\n";

	// Initialize with Two-Pass, then switch to one-pass
	using DefaultOptions =
	    TreeOptions<TreeFlags::MULTIPLE, TreeFlags::WBT_DELTA_NUMERATOR<3>,
	                TreeFlags::WBT_DELTA_DENOMINATOR<1>,
	                TreeFlags::WBT_GAMMA_NUMERATOR<4>,
	                TreeFlags::WBT_GAMMA_DENOMINATOR<3>>;
	using SPOptions = TreeOptions<
	    TreeFlags::MULTIPLE, TreeFlags::WBT_SINGLE_PASS,
	    TreeFlags::WBT_DELTA_NUMERATOR<3>, TreeFlags::WBT_DELTA_DENOMINATOR<1>,
	    TreeFlags::WBT_GAMMA_NUMERATOR<4>, TreeFlags::WBT_GAMMA_DENOMINATOR<3>>;
	BalanceAnalyzer<DefaultOptions, SPOptions> ba(node_count, reinsertion_count,
	                                              series_os, distr_os);
	for (int seed = 42; seed < 42 + seed_count; ++seed) {
		std::cout << "Seed: " << seed << "\n";
		ba.run(seed);
	}
	ba.print();

	/*

	// Standard WBTree
	using DefaultOptions = TreeOptions<TreeFlags::MULTIPLE>;
	BalanceAnalyzer<DefaultOptions> default_ba(node_count, reinsertion_count);
	for (int seed = 42; seed < 42 + seed_count; ++seed) {
	  default_ba.run(seed);
	}
	default_ba.print();

	// 3,2-WBTree
	using WBT32Options = TreeOptions<TreeFlags::MULTIPLE,
	TreeFlags::WBT_DELTA_NUMERATOR<3>, TreeFlags::WBT_DELTA_DENOMINATOR<1>,
	TreeFlags::WBT_GAMMA_NUMERATOR<2>, TreeFlags::WBT_GAMMA_DENOMINATOR<1>>;
	BalanceAnalyzer<WBT32Options> wbt32_ba(node_count, reinsertion_count);
	for (int seed = 42; seed < 42 + seed_count; ++seed) {
	  wbt32_ba.run(seed);
	}
	wbt32_ba.print();

	  // Single-Pass WBTree
	using SPOptions = TreeOptions<TreeFlags::MULTIPLE,
	TreeFlags::WBT_SINGLE_PASS>; BalanceAnalyzer<SPOptions> sp_ba(node_count,
	reinsertion_count); for (int seed = 42; seed < 42 + seed_count; ++seed) {
	  sp_ba.run(seed);
	}
	sp_ba.print();

	*/
	return 0;
}
