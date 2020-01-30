#include "../src/dynamic_segment_tree.hpp"
#include "../src/options.hpp"

#include <random>
#include <vector>

template <class Tree, class Node>
class BalanceAnalyzer {
public:
	BalanceAnalyzer(size_t nodecount, int seed)
	    : nodes(nodecount), rnd(static_cast<unsigned long>(seed))
	{}

	void
	run() noexcept
	{
		this->build();
		this->rebuild();
		this->compute_statistics();
	}

	double
	get_average_depth() const noexcept
	{
		return static_cast<double>(this->depth_sum) /
		       static_cast<double>(this->nodes.size());
	}

	size_t
	get_max_depth() const noexcept
	{
		return this->max_depth;
	}

private:
	void
	compute_statistics()
	{
		this->depth_sum = 0;
		this->max_depth = 0;

		for (const auto & node : this->t) {
			size_t depth = node.get_depth();
			this->max_depth = std::max(depth, this->max_depth);
			this->depth_sum += depth;
		}
	}

	void
	build()
	{
		for (auto & node : nodes) {
			std::uniform_int_distribution bound_distr(
			    std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
			node.lower = bound_distr(this->rnd);
			node.upper = bound_distr(this->rnd);
			node.value = 1;

			if (node.lower > node.upper) {
				std::swap(node.lower, node.upper);
			}
			if (node.lower == node.upper) {
				node.upper++;
			}

			this->t.insert(node);
		}
	}

	void
	rebuild()
	{
		for (auto & node : nodes) {
			std::uniform_int_distribution bound_distr(
			    std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

			this->t.remove(node);

			node.lower = bound_distr(this->rnd);
			node.upper = bound_distr(this->rnd);

			if (node.lower > node.upper) {
				std::swap(node.lower, node.upper);
			}
			if (node.lower == node.upper) {
				node.upper++;
			}

			this->t.insert(node);
		}
	}

	std::vector<Node> nodes;
	Tree t;
	std::mt19937 rnd;

	size_t depth_sum;
	size_t max_depth;
};
/*
 * General Node + Traits
 */
template <class Selector>
class Node
    : public ygg::DynSegTreeNodeBase<
          int, double, double, ygg::EmptyCombinerPack<int, double>, Selector> {
public:
	int lower;
	int upper;
	double value;
};
template <class MyNode>
class NodeTraits : public ygg::DynSegTreeNodeTraits<MyNode> {
public:
	using Node = MyNode;

	static int
	get_lower(const Node & n)
	{
		return n.lower;
	}
	static int
	get_upper(const Node & n)
	{
		return n.upper;
	}
	static double
	get_value(const Node & n)
	{
		return n.value;
	}
};

/*
 * RB-Tree
 */
using RBTOptions = ygg::TreeOptions<ygg::TreeFlags::MULTIPLE>;
using RBTSelector = ygg::UseDefaultRBTree;
using RBDSTNode = Node<RBTSelector>;
using RBDSTNodeTraits = NodeTraits<RBDSTNode>;

using RBTTree = ygg::DynamicSegmentTree<RBDSTNode, RBDSTNodeTraits,
                                        ygg::EmptyCombinerPack<int, double>,
                                        RBTOptions, RBTSelector>;

/*
 * Zip Tree using hashing
 */
using ZTHOptions = ygg::TreeOptions<ygg::TreeFlags::MULTIPLE>;
using ZTHSelector =
    ygg::UseZipTree<ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT<
                        16186402584962403883ul>,
                    ygg::TreeFlags::ZTREE_USE_HASH>;
using ZHDSTNode = Node<ZTHSelector>;
using ZHDSTNodeTraits = NodeTraits<ZHDSTNode>;

using ZHTree = ygg::DynamicSegmentTree<ZHDSTNode, ZHDSTNodeTraits,
                                       ygg::EmptyCombinerPack<int, double>,
                                       ZTHOptions, ZTHSelector>;

/*
 * Zip Tree using randomnes
 */
using ZTROptions = ygg::TreeOptions<ygg::TreeFlags::MULTIPLE>;
using ZTRSelector =
    ygg::UseZipTree<ygg::TreeFlags::ZTREE_RANK_TYPE<std::uint8_t>>;
using ZRDSTNode = Node<ZTRSelector>;
using ZRDSTNodeTraits = NodeTraits<ZRDSTNode>;

using ZRTree = ygg::DynamicSegmentTree<ZRDSTNode, ZRDSTNodeTraits,
                                       ygg::EmptyCombinerPack<int, double>,
                                       ZTROptions, ZTRSelector>;

int
main(int argc, char ** argv)
{
	std::string out_fname = argv[1];
	std::ofstream out_stream(out_fname, std::ios::trunc);
	out_stream << "Name,Seed,Size,AvgDepth,MaxDepth\n";

	size_t doublings = 15;
	size_t base_size = 1000;
	size_t seed_start = 42;
	size_t seed_count = 50;

	for (size_t seed = seed_start; seed < seed_start + seed_count; seed++) {
		for (size_t doubling = 1; doubling <= doublings; doubling++) {
			size_t size = base_size * std::pow(2, doubling);

			BalanceAnalyzer<RBTTree, RBDSTNode> rbt_analyzer(size, seed);
			rbt_analyzer.run();
			out_stream << "rbt," << seed << "," << size << ","
			           << rbt_analyzer.get_average_depth() << ","
			           << rbt_analyzer.get_max_depth() << "\n";

			BalanceAnalyzer<ZHTree, ZHDSTNode> zh_analyzer(size, seed);
			zh_analyzer.run();
			out_stream << "zh," << seed << "," << size << ","
			           << zh_analyzer.get_average_depth() << ","
			           << zh_analyzer.get_max_depth() << "\n";

			BalanceAnalyzer<ZRTree, ZRDSTNode> zr_analyzer(size, seed);
			zr_analyzer.run();
			out_stream << "zr," << seed << "," << size << ","
			           << zr_analyzer.get_average_depth() << ","
			           << zr_analyzer.get_max_depth() << "\n";

			out_stream << std::flush;
		}
	}
}
