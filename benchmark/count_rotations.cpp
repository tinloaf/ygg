#include "../src/options.hpp"
#include "../src/ygg.hpp"
#include <algorithm>
#include <random>

using namespace ygg;

class CountingNodeTraits : public weight::WBDefaultNodeTraits {
public:
	template <class Node, class Tree>
	static void
	rotated_left(Node & node, Tree & t)
	{
		(void)node;
		(void)t;

		CountingNodeTraits::count++;
		CountingNodeTraits::size += node._wbt_size;
	}

	template <class Node, class Tree>
	static void
	rotated_right(Node & node, Tree & t)
	{
		(void)node;
		(void)t;

		CountingNodeTraits::size += node._wbt_size;
		CountingNodeTraits::count++;
	}

	static void
	reset()
	{
		CountingNodeTraits::count = 0;
		CountingNodeTraits::size = 0;
	}

	static size_t
	get_count()
	{
		return CountingNodeTraits::count;
	}

	static size_t
	get_size()
	{
		return CountingNodeTraits::size;
	}

	static size_t count;
	static size_t size;
};

class RBCountingNodeTraits : public RBDefaultNodeTraits {
public:
	template <class Node>
	static size_t
	compute_size(Node & n)
	{
		size_t res = 0;
		if (n.get_left() == nullptr) {
			res += 1;
		} else {
			res += compute_size(*n.get_left());
		}

		if (n.get_right() == nullptr) {
			res += 1;
		} else {
			res += compute_size(*n.get_right());
		}

		return res;
	}

	template <class Node, class Tree>
	static void
	rotated_left(Node & node, Tree & t)
	{
		(void)node;
		(void)t;

		CountingNodeTraits::count++;
		CountingNodeTraits::size += compute_size(node);
	}

	template <class Node, class Tree>
	static void
	rotated_right(Node & node, Tree & t)
	{
		(void)node;
		(void)t;

		CountingNodeTraits::size += compute_size(node);
		CountingNodeTraits::count++;
	}

	static void
	reset()
	{
		CountingNodeTraits::count = 0;
		CountingNodeTraits::size = 0;
	}

	static size_t
	get_count()
	{
		return CountingNodeTraits::count;
	}

	static size_t
	get_size()
	{
		return CountingNodeTraits::size;
	}

	static size_t count;
	static size_t size;
};

size_t CountingNodeTraits::count = 0;
size_t CountingNodeTraits::size = 0;
size_t RBCountingNodeTraits::count = 0;
size_t RBCountingNodeTraits::size = 0;

template <class Flags>
class RotationCounter {
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

	using Tree = weight::WBTree<Node<Flags>, CountingNodeTraits, Flags>;

public:
	RotationCounter(std::string name_in, size_t node_count_in,
	                size_t operation_count_in, std::ofstream & outfile_in)

	    : name(name_in), node_count(node_count_in),
	      operation_count(operation_count_in), outfile(outfile_in)
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

		CountingNodeTraits::reset();
		for (size_t i = 0; i < this->operation_count; ++i) {
			size_t index = (size_t)(std::abs(distr(rnd))) % this->node_count;

			// TODO optimistic - also needs distinct values!
			// this->init_t.remove(*(Node<InitialFlags> *)(&this->nodes[index]));
			this->t.remove(this->nodes[index]);
			//			assert(this->t->dbg_count_violations() == 0);
			this->nodes[index].key = distr(rnd);
			this->t.insert(this->nodes[index]);
		}

		this->outfile << this->name << "," << seed << "," << this->node_count << ","
		              << this->operation_count << ","
		              << CountingNodeTraits::get_count() << ","
		              << CountingNodeTraits::get_size() << "\n";
	}

private:
	std::string name;
	size_t node_count;
	size_t operation_count;
	std::vector<Node<Flags>> nodes;
	Tree t;
	std::ofstream & outfile;
};

class RBRotationCounter {
private:
	class Node : public RBTreeNodeBase<Node, DefaultOptions> {
	public:
		int key;

		bool
		operator<(const Node & other) const
		{
			return this->key < other.key;
		}
	};

	using Tree = RBTree<Node, RBCountingNodeTraits, DefaultOptions>;

public:
	RBRotationCounter(std::string name_in, size_t node_count_in,
	                  size_t operation_count_in, std::ofstream & outfile_in)

	    : name(name_in), node_count(node_count_in),
	      operation_count(operation_count_in), outfile(outfile_in)
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

		RBCountingNodeTraits::reset();
		for (size_t i = 0; i < this->operation_count; ++i) {
			size_t index = (size_t)(std::abs(distr(rnd))) % this->node_count;

			// TODO optimistic - also needs distinct values!
			// this->init_t.remove(*(Node<InitialFlags> *)(&this->nodes[index]));
			this->t.remove(this->nodes[index]);
			//			assert(this->t->dbg_count_violations() == 0);
			this->nodes[index].key = distr(rnd);
			this->t.insert(this->nodes[index]);
		}

		this->outfile << this->name << "," << seed << "," << this->node_count << ","
		              << this->operation_count << ","
		              << RBCountingNodeTraits::get_count() << ","
		              << RBCountingNodeTraits::get_size() << "\n";
	}

private:
	std::string name;
	size_t node_count;
	size_t operation_count;
	std::vector<Node> nodes;
	Tree t;
	std::ofstream & outfile;
};

int
main(int argc, char ** argv)
{
	(void)argc;
	size_t node_count = (size_t)atoi(argv[1]);
	size_t operation_count_base = (size_t)atoi(argv[2]);
	size_t operation_count_increment = (size_t)atoi(argv[3]);
	size_t operation_count_steps = (size_t)atoi(argv[4]);
	size_t seed_count = (size_t)atoi(argv[5]);

	// CSV file to write gradual counts to
	std::string out_dir(argv[6]);
	std::string out_fname = out_dir + "/rotations.csv";
	std::ofstream outfile(out_fname, std::ios::trunc);
	outfile << "name,seed,nodecount,opcount,rotations,size\n";

	using DefOptions = TreeOptions<TreeFlags::MULTIPLE>;
	using SPOptions =
	    TreeOptions<TreeFlags::MULTIPLE, TreeFlags::WBT_SINGLE_PASS>;
	using SP32Options = TreeOptions<
	    TreeFlags::MULTIPLE, TreeFlags::WBT_SINGLE_PASS,
	    TreeFlags::WBT_DELTA_NUMERATOR<3>, TreeFlags::WBT_DELTA_DENOMINATOR<1>,
	    TreeFlags::WBT_GAMMA_NUMERATOR<2>, TreeFlags::WBT_GAMMA_DENOMINATOR<1>>;
	using SPBalOptions = TreeOptions<
	    TreeFlags::MULTIPLE, TreeFlags::WBT_SINGLE_PASS,
	    TreeFlags::WBT_DELTA_NUMERATOR<2>, TreeFlags::WBT_DELTA_DENOMINATOR<1>,
	    TreeFlags::WBT_GAMMA_NUMERATOR<3>, TreeFlags::WBT_GAMMA_DENOMINATOR<2>>;
	using SPLWOptions =
	    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE,
	                     ygg::TreeFlags::WBT_SINGLE_PASS,
	                     ygg::TreeFlags::WBT_DELTA_NUMERATOR<3>,
	                     ygg::TreeFlags::WBT_DELTA_DENOMINATOR<1>,
	                     ygg::TreeFlags::WBT_GAMMA_NUMERATOR<4>,
	                     ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<3>>;

	for (size_t i = 0; i < operation_count_steps; ++i) {
		size_t opcount = operation_count_base + (i * operation_count_increment);

		RBRotationCounter rcRB("RBTree", node_count, opcount, outfile);
		for (size_t seed = 42; seed < 42 + seed_count; ++seed) {
			std::cout << "Seed: " << seed << "\n";
			rcRB.run((int)seed);
		}

		RotationCounter<DefOptions> rcTP("WBTree[Default|TP]", node_count, opcount,
		                                 outfile);
		for (size_t seed = 42; seed < 42 + seed_count; ++seed) {
			std::cout << "Seed: " << seed << "\n";
			rcTP.run((int)seed);
		}

		RotationCounter<SPOptions> rcSP("WBTree[Default|SP]", node_count, opcount,
		                                outfile);
		for (size_t seed = 42; seed < 42 + seed_count; ++seed) {
			std::cout << "Seed: " << seed << "\n";
			rcSP.run((int)seed);
		}

		RotationCounter<SP32Options> rc32("WBTree[3|2|SP]", node_count, opcount,
		                                  outfile);
		for (size_t seed = 42; seed < 42 + seed_count; ++seed) {
			std::cout << "Seed: " << seed << "\n";
			rc32.run((int)seed);
		}

		RotationCounter<SPBalOptions> rcBal("WBTree[2|3/2|SP]", node_count, opcount,
		                                    outfile);
		for (size_t seed = 42; seed < 42 + seed_count; ++seed) {
			std::cout << "Seed: " << seed << "\n";
			rcBal.run((int)seed);
		}

		RotationCounter<SPLWOptions> rcLW("WBTree[3|4/3|SP]", node_count, opcount,
		                                  outfile);
		for (size_t seed = 42; seed < 42 + seed_count; ++seed) {
			std::cout << "Seed: " << seed << "\n";
			rcLW.run((int)seed);
		}
	}

	return 0;
}
