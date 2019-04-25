#include "../src/ygg.hpp"

#include <fstream>
#include <iostream>
#include <random>
#include <tuple>
#include <vector>

template <class Tree, class Node>
class TreeDepthAnalyzer {
public:
	TreeDepthAnalyzer(std::string name_in, size_t count_in, size_t move_count_in,
	                  size_t seed_in, std::ofstream & os_in)
	    : name(name_in), count(count_in), move_count(move_count_in),
	      seed(seed_in), os(os_in){};

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

		this->os << this->name << "," << this->count << "," << this->move_count
		         << "," << this->seed << ","
		         << this->path_lengths[this->path_lengths.size() / 2] << ","
		         << ((double)sum) / ((double)this->count) << "," << sum << ","
		         << *std::max_element(this->path_lengths.begin(),
		                              this->path_lengths.end())
		         << "\n";
	}

private:
	std::string name;
	size_t count;
	size_t move_count;
	size_t seed;
	std::ofstream & os;

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
		std::uniform_int_distribution<size_t> distr(
		    0, std::numeric_limits<size_t>::max());

		this->nodes.resize(this->count);
		std::set<size_t> values_seen;
		for (unsigned int i = 0; i < this->count; ++i) {
			size_t val = distr(rnd);
			while (values_seen.find(val) != values_seen.end()) {
				val = distr(rnd);
			}
			values_seen.insert(val);

			this->nodes[i].val = val;
			this->t.insert(this->nodes[i]);
		}

		std::vector<size_t> move_indices(this->count);
		std::iota(move_indices.begin(), move_indices.end(), 0);
		std::shuffle(move_indices.begin(), move_indices.end(), rnd);

		for (size_t i = 0; i < this->move_count; ++i) {
			this->t.erase(this->nodes[move_indices[i]]); // TODO erase optimistic!

			size_t val = distr(rnd);
			while (values_seen.find(val) != values_seen.end()) {
				val = distr(rnd);
			}
			values_seen.insert(val);

			this->nodes[move_indices[i]].val = val;
			this->t.insert(this->nodes[move_indices[i]]);
		}
	}
};

using BasicTreeOptions =
    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE, ygg::TreeFlags::ZTREE_USE_HASH,
                     ygg::TreeFlags::ZTREE_RANK_TYPE<uint8_t>,
                     ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT<
                         9859957398433823229ul>,
                     ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_MODUL<
                         std::numeric_limits<size_t>::max()>>;

using RandomRankTreeOptions =
    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE,
                     ygg::TreeFlags::ZTREE_RANK_TYPE<uint8_t>,
                     ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT<
                         9859957398433823229ul>,
                     ygg::TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_MODUL<
                         std::numeric_limits<size_t>::max()>>;

/* WBTree Options */
using WBTTwopassTreeOptions = ygg::TreeOptions<ygg::TreeFlags::MULTIPLE>;
using WBTSinglepassTreeOptions =
    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE, ygg::TreeFlags::WBT_SINGLE_PASS>;

using WBTTwopass32TreeOptions =
    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE,
                     ygg::TreeFlags::WBT_DELTA_NUMERATOR<3>,
                     ygg::TreeFlags::WBT_DELTA_DENOMINATOR<1>,
                     ygg::TreeFlags::WBT_GAMMA_NUMERATOR<2>,
                     ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<1>>;
using WBTSinglepass32TreeOptions =
    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE, ygg::TreeFlags::WBT_SINGLE_PASS,
                     ygg::TreeFlags::WBT_DELTA_NUMERATOR<3>,
                     ygg::TreeFlags::WBT_DELTA_DENOMINATOR<1>,
                     ygg::TreeFlags::WBT_GAMMA_NUMERATOR<2>,
                     ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<1>>;

using WBTSinglepassLWTreeOptions =
    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE, ygg::TreeFlags::WBT_SINGLE_PASS,
                     ygg::TreeFlags::WBT_DELTA_NUMERATOR<3>,
                     ygg::TreeFlags::WBT_DELTA_DENOMINATOR<1>,
                     ygg::TreeFlags::WBT_GAMMA_NUMERATOR<4>,
                     ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<3>>;

using WBTSinglepassBalTreeOptions =
    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE, ygg::TreeFlags::WBT_SINGLE_PASS,
                     ygg::TreeFlags::WBT_DELTA_NUMERATOR<2>,
                     ygg::TreeFlags::WBT_DELTA_DENOMINATOR<1>,
                     ygg::TreeFlags::WBT_GAMMA_NUMERATOR<3>,
                     ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<2>>;

using WBTSinglepassMostBalTreeOptions =
    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE, ygg::TreeFlags::WBT_SINGLE_PASS,
                     ygg::TreeFlags::WBT_DELTA_NUMERATOR<3>,
                     ygg::TreeFlags::WBT_DELTA_DENOMINATOR<2>,
                     ygg::TreeFlags::WBT_GAMMA_NUMERATOR<1>,
                     ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<1>>;

class RBTreeNode : public ygg::RBTreeNodeBase<RBTreeNode, BasicTreeOptions> {
public:
	size_t val;
};

bool
operator<(const RBTreeNode & lhs, const RBTreeNode & rhs)
{
	return lhs.val < rhs.val;
}

template <class Options>
class WBTreeNode
    : public ygg::weight::WBTreeNodeBase<WBTreeNode<Options>, Options> {
public:
	size_t val;

	bool
	operator<(const WBTreeNode & other) const
	{
		return this->val < other.val;
	}
};

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

template <class T>
struct type_container
{
	using type = T;
};

/* RBTree */
using RBTree =
    ygg::RBTree<RBTreeNode, ygg::RBDefaultNodeTraits, BasicTreeOptions>;

/* WBTree */
template <class Options>
using WBTree = ygg::weight::WBTree<WBTreeNode<Options>,
                                   ygg::weight::WBDefaultNodeTraits, Options>;

/* Energy-Balanced Tree */
using EnergyTree = ygg::EnergyTree<EnergyNode, BasicTreeOptions>;

/* ZTree */
using ZTree = ygg::ZTree<ZTreeNode, ygg::ZTreeDefaultNodeTraits<ZTreeNode>,
                         BasicTreeOptions>;

/* RandZTree */
using RandZTree =
    ygg::ZTree<RandZTreeNode, ygg::ZTreeDefaultNodeTraits<RandZTreeNode>,
               RandomRankTreeOptions>;

auto
all_types()
{
	return std::make_tuple(
	    std::make_tuple(std::string("RBTree"), type_container<RBTree>{},
	                    type_container<RBTreeNode>{}),
	    std::make_tuple(std::string("WBTree[TP]"),
	                    type_container<WBTree<WBTTwopassTreeOptions>>{},
	                    type_container<WBTreeNode<WBTTwopassTreeOptions>>{}),
	    std::make_tuple(std::string("WBTree[SP]"),
	                    type_container<WBTree<WBTSinglepassTreeOptions>>{},
	                    type_container<WBTreeNode<WBTSinglepassTreeOptions>>{}),
	    /*	    std::make_tuple(std::string("WBTree[TP|32]"),
	                    type_container<WBTree<WBTTwopass32TreeOptions>>{},
	                    type_container<WBTreeNode<WBTTwopass32TreeOptions>>{}),*/
	    std::make_tuple(std::string("WBTree[SP|32]"),
	                    type_container<WBTree<WBTSinglepass32TreeOptions>>{},
	                    type_container<WBTreeNode<WBTSinglepass32TreeOptions>>{}),
	    std::make_tuple(std::string("WBTree[SP|LW]"),
	                    type_container<WBTree<WBTSinglepassLWTreeOptions>>{},
	                    type_container<WBTreeNode<WBTSinglepassLWTreeOptions>>{}),
	    std::make_tuple(
	        std::string("WBTree[SP|Bal]"),
	        type_container<WBTree<WBTSinglepassBalTreeOptions>>{},
	        type_container<WBTreeNode<WBTSinglepassBalTreeOptions>>{}),
	    std::make_tuple(
	        std::string("WBTree[SP|MostBal]"),
	        type_container<WBTree<WBTSinglepassMostBalTreeOptions>>{},
	        type_container<WBTreeNode<WBTSinglepassMostBalTreeOptions>>{})

	);
}

template <std::size_t I = 0, typename... Tpl>
typename std::enable_if<I == sizeof...(Tpl), void>::type
do_analysis(std::tuple<Tpl...> tpl, size_t count, size_t move_count,
            size_t seed_count, size_t seed_start, std::ofstream & os)
{
	(void)tpl;
	(void)count;
	(void)move_count;
	(void)seed_count;
	(void)seed_start;
	(void)os;
}

template <std::size_t I = 0, typename... Tpl>
    typename std::enable_if <
    I<sizeof...(Tpl), void>::type
    do_analysis(std::tuple<Tpl...> tpl, size_t count, size_t move_count,
                size_t seed_count, size_t seed_start, std::ofstream & os)
{
	auto & el = std::get<I>(tpl);

	std::string name = std::get<0>(el);
	using TreeClass =
	    typename std::remove_reference<decltype(std::get<1>(el))>::type::type;
	using NodeClass =
	    typename std::remove_reference<decltype(std::get<2>(el))>::type::type;

	std::cout << "================== " << name << "\n";
	for (size_t seed = seed_start; seed < seed_start + seed_count; ++seed) {
		TreeDepthAnalyzer<TreeClass, NodeClass> tda(name, count, move_count, seed,
		                                            os);
		tda.run();
	}

	do_analysis<I + 1, Tpl...>(tpl, count, move_count, seed_count, seed_start,
	                           os);
}

int
main(int argc, const char ** argv)
{
	(void)argc; // TODO print an error message if wrong

	size_t base_count = (size_t)std::atol(argv[1]);
	size_t offset = (size_t)std::atol(argv[2]);
	size_t additions = (size_t)std::atol(argv[3]);
	double move_fraction = (double)std::atof(argv[4]);
	size_t seed_start = (size_t)std::atol(argv[5]);
	size_t seed_count = (size_t)std::atol(argv[6]);
	std::string file_name(argv[7]);

	std::ofstream os(file_name, std::ios::trunc);

	// Write header
	os << "name,size,move_count,seed,median_depth,average_depth,depth_sum,max_"
	      "depth\n";

	for (size_t a = 0; a <= additions; ++a) {
		std::cout << "################### " << a << " / " << additions << "\n";
		size_t count = base_count + (a * offset);
		size_t move_count = (size_t)(count * move_fraction);

		do_analysis(all_types(), count, move_count, seed_count, seed_start, os);
	}
}
