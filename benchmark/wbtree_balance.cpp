#include "../src/wbtree.hpp"
#include "/home/lukas/src/intervaltree/ygg/src/options.hpp"
#include <algorithm>
#include <random>

using namespace ygg;

template <class Flags>
class BalanceAnalyzer {
private:
	class Node : public weight::WBTreeNodeBase<Node, Flags> {
	public:
		int key;

		bool
		operator<(const Node & other) const
		{
			return this->key < other.key;
		}
	};

	using Tree = weight::WBTree<Node, weight::WBDefaultNodeTraits, Flags>;

public:
	BalanceAnalyzer(size_t node_count_in, size_t reinsertion_count_in)
	    : node_count(node_count_in), reinsertion_count(reinsertion_count_in)
	{
		this->nodes.resize(this->node_count);
	}

	void
	run(int seed)
	{
		std::mt19937 rnd(seed);
		std::uniform_int_distribution distr(std::numeric_limits<int>::min(),
		                                    std::numeric_limits<int>::max());

		this->t.clear();

		for (size_t i = 0; i < this->node_count; ++i) {
			this->nodes[i].key = distr(rnd);
			this->t.insert(this->nodes[i]);
		}

		for (size_t i = 0; i < this->reinsertion_count; ++i) {
			size_t index = std::abs(distr(rnd)) % this->node_count;

			// TODO optimistic - also needs distinct values!
			this->t.remove(this->nodes[index]);
			this->nodes[index].key = distr(rnd);
			this->t.insert(this->nodes[index]);
		}

		this->counts.push_back(this->t.dbg_count_violations());
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
	std::vector<Node> nodes;
	std::vector<size_t> counts;
	Tree t;
};

int
main(int argc, char ** argv)
{
	size_t node_count = atoi(argv[1]);
	size_t reinsertion_count = atoi(argv[2]);
	size_t seed_count = atoi(argv[3]);

	// Standard WBTree
	using DefaultOptions = TreeOptions<TreeFlags::MULTIPLE>;
	BalanceAnalyzer<DefaultOptions> default_ba(node_count, reinsertion_count);
	for (int seed = 42; seed < 42 + seed_count; ++seed) {
		default_ba.run(seed);
	}
	default_ba.print();

	return 0;
}
