#include "../src/ygg.hpp"

#include <iostream>
#include <random>
#include <vector>

template <class Tree, class Node>
class TreeDepthAnalyzer {
public:
	TreeDepthAnalyzer(size_t count_in, size_t seed_in)
	    : count(count_in), seed(seed_in){};

	void
	run()
	{
		this->create_nodes();
		this->compute_path_lengths();

		std::sort(this->path_lengths.begin(), this->path_lengths.end());

		std::cout << "Median Depth: \t\t"
		          << this->path_lengths[this->path_lengths.size() / 2] << std::endl;
		size_t sum = (size_t)std::accumulate(this->path_lengths.begin(),
		                                     this->path_lengths.end(), 0);
		std::cout << "Average Depth: \t\t" << ((double)sum) / ((double)this->count)
		          << std::endl;
		std::cout << "Depth Sum: \t\t" << sum << std::endl;
		std::cout << "Maximum Depth: \t\t"
		          << *std::max_element(this->path_lengths.begin(),
		                               this->path_lengths.end())
		          << std::endl;

		double balanced_depth = std::floor(std::log2(this->count));
		size_t deeper_than_balanced = 0;
		for (size_t depth : this->path_lengths) {
			if (depth > balanced_depth) {
				deeper_than_balanced++;
			}
		}
		std::cout << "Vertices too Deep: \t" << deeper_than_balanced << std::endl;
	}

private:
	size_t count;
	size_t seed;

	Tree t;
	std::vector<Node> nodes;
	std::vector<size_t> path_length_histogram;
	std::vector<size_t> path_lengths;
	
	void
	compute_path_lengths()
	{
		for (const auto & n : this->t) {
			size_t depth = n.get_depth();
			if (this->path_length_histogram.size() < depth + 1) {
				this->path_length_histogram.resize(depth + 1, 0);
			}
			this->path_length_histogram[depth]++;
			this->path_lengths.push_back(depth);
		}
	}

	void
	create_nodes()
	{
		std::mt19937 rnd(this->seed);
		std::uniform_int_distribution<size_t> rng(
		    0, std::numeric_limits<size_t>::max());
		this->nodes.resize(this->count);
		for (unsigned int i = 0; i < this->count; ++i) {
			this->nodes[i].val = rng(rnd);
			this->t.insert(this->nodes[i]);
		}
	}
};

using BasicTreeOptions =
    ygg::TreeOptions<ygg::TreeFlags::ZTREE_USE_HASH,
                     ygg::TreeFlags::ZTREE_RANK_TYPE<uint8_t>,
                     ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT<
                         9859957398433823229ul>,
                     ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_MODUL<
                         std::numeric_limits<size_t>::max()>>;

using RandomRankTreeOptions =
    ygg::TreeOptions<ygg::TreeFlags::ZTREE_RANK_TYPE<uint8_t>,
                     ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT<
                         9859957398433823229ul>,
                     ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_MODUL<
                         std::numeric_limits<size_t>::max()>>;

class RBTreeNode : public ygg::RBTreeNodeBase<RBTreeNode, BasicTreeOptions> {
public:
	size_t val;
};

bool
operator<(const RBTreeNode & lhs, const RBTreeNode & rhs)
{
	return lhs.val < rhs.val;
}

class WBTreeNode
    : public ygg::weight::WBTreeNodeBase<WBTreeNode, BasicTreeOptions> {
public:
	size_t val;
};

bool
operator<(const WBTreeNode & lhs, const WBTreeNode & rhs)
{
	return lhs.val < rhs.val;
}

class EnergyNode
    : public ygg::EnergyTreeNodeBase<EnergyNode, BasicTreeOptions> {
public:
	size_t val;
};

bool
operator<(const EnergyNode & lhs, const EnergyNode & rhs)
{
	return lhs.val < rhs.val;
}

class ZTreeNode : public ygg::ZTreeNodeBase<ZTreeNode, BasicTreeOptions> {
public:
	size_t val;
};

class RandZTreeNode
    : public ygg::ZTreeNodeBase<RandZTreeNode, RandomRankTreeOptions> {
public:
	size_t val;
};

bool
operator<(const ZTreeNode & lhs, const ZTreeNode & rhs)
{
	return lhs.val < rhs.val;
}

bool
operator<(const RandZTreeNode & lhs, const RandZTreeNode & rhs)
{
	return lhs.val < rhs.val;
}

int
main(int argc, const char ** argv)
{
	(void)argc; // TODO print an error message if wrong

	/* RBTree */
	using RBTree =
	    ygg::RBTree<RBTreeNode, ygg::RBDefaultNodeTraits, BasicTreeOptions>;

	/* WBTree */
	using WBTree = ygg::weight::WBTree<WBTreeNode, ygg::RBDefaultNodeTraits,
	                                   BasicTreeOptions>;

	/* Energy-Balanced Tree */
	using EnergyTree = ygg::EnergyTree<EnergyNode, BasicTreeOptions>;

	/* ZTree */
	using ZTree = ygg::ZTree<ZTreeNode, ygg::ZTreeDefaultNodeTraits<ZTreeNode>,
	                         BasicTreeOptions>;

	/* RandZTree */
	using RandZTree =
	    ygg::ZTree<RandZTreeNode, ygg::ZTreeDefaultNodeTraits<RandZTreeNode>,
	               RandomRankTreeOptions>;

	size_t count = (size_t)std::atol(argv[1]);
	size_t seed = (size_t)std::atol(argv[2]);

	std::cout << "==== Red-Black Tree ====\n";
	TreeDepthAnalyzer<RBTree, RBTreeNode> tarb(count, seed);
	tarb.run();

	std::cout << "==== Weight-Balanced Tree ====\n";
	TreeDepthAnalyzer<WBTree, WBTreeNode> taw(count, seed);
	taw.run();

	std::cout << "==== Energy-Balanced Tree ====\n";
	TreeDepthAnalyzer<EnergyTree, EnergyNode> tae(count, seed);
	tae.run();

	std::cout << "==== Zip Tree ====\n";
	TreeDepthAnalyzer<ZTree, ZTreeNode> taz(count, seed);
	taz.run();

	std::cout << "==== Random Zip Tree ====\n";
	TreeDepthAnalyzer<RandZTree, RandZTreeNode> rtaz(count, seed);
	rtaz.run();
}
