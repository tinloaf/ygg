#include "../src/benchmark_sequence.hpp"
#include "../src/options.hpp"
#include "../src/ygg.hpp"

#include <cmath>
#include <fstream>

using namespace ygg;

#include <chrono>
#include <numeric>
#include <ostream>
#include <type_traits>
#include <unordered_map>

template <class Tree, class Node>
class DepthAnalyzer {
public:
	DepthAnalyzer(const Tree & t_in) : t(t_in){};

	void
	compute_path_lengths()
	{
		this->node_count = 0;
		this->path_length_histogram.clear();
		this->path_lengths.clear();

		for (const auto & n : this->t) {
			this->node_count++;
			size_t depth = n.get_depth();
			if (this->path_length_histogram.size() < depth + 1) {
				this->path_length_histogram.resize(depth + 1, 0);
			}
			this->path_length_histogram[depth]++;
			this->path_lengths.push_back(depth);
		}

		std::sort(this->path_lengths.begin(), this->path_lengths.end());
		this->path_length_sum = std::accumulate(
		    this->path_lengths.begin(), this->path_lengths.end(), size_t{0});
		this->max_depth = this->path_lengths.back();
		if (this->node_count % 2 == 0) {
			this->median_depth =
			    static_cast<double>(
			        (this->path_lengths[this->node_count / 2] +
			         this->path_lengths[(this->node_count / 2) - 1])) /
			    2.0;
		} else {
			this->median_depth = this->path_lengths[(this->node_count - 1) / 2];
		}
	}

	double
	get_average() const noexcept
	{
		return static_cast<double>(this->path_length_sum) /
		       static_cast<double>(this->node_count);
	}

	double
	get_median() const noexcept
	{
		return this->median_depth;
	}

	size_t
	get_maximum() const noexcept
	{
		return this->max_depth;
	}

private:
	size_t node_count;
	std::vector<size_t> path_length_histogram;
	std::vector<size_t> path_lengths;
	double median_depth;
	size_t path_length_sum;
	size_t max_depth;

	const Tree & t;
};

template <class Tree, class Node, size_t depth_interval>
class SequenceBenchmarker {
private:
	constexpr static size_t CHUNKSIZE = 100000;
	using KeyT = unsigned int;

public:
	SequenceBenchmarker(std::string filename_in, std::string benchmark_name_in,
	                    std::string name_in, size_t repetitions_in,
	                    std::ostream & out_in)
	    : reader(filename_in.c_str()), filename(filename_in),
	      benchmark_name(benchmark_name_in), name(name_in),
	      repetitions(repetitions_in), out(out_in)
	{
		this->create_nodes();
		this->run();
		this->cleanup();
	};

private:
	void
	cleanup()
	{
		this->nodes.clear();
		this->nodes.shrink_to_fit();
	}

	void
	run()
	{
		while (this->repetitions >= 1) {
			double elapsed_round = 0;
			this->reader.reset();
			Tree t;
			DepthAnalyzer<Tree, Node> da(t);

			std::cerr << this->repetitions << " iterations remaining...\n";

			for (auto & buf = this->reader.get(CHUNKSIZE); buf.size() > 0;
			     this->reader.get(CHUNKSIZE)) {
				auto started_at = std::chrono::high_resolution_clock::now();

				size_t i = 0;

				for (auto & entry : buf) {
					if constexpr (depth_interval > 0) {
						i++;

						if (i % depth_interval == 0) {
							da.compute_path_lengths();
							this->out << this->benchmark_name << "," << this->name << ","
							          << this->repetitions << "," << i << ","
							          << da.get_average() << "," << da.get_median() << ","
							          << da.get_maximum() << "\n";
						}
					}

					Node & n = this->nodes[reinterpret_cast<size_t>(entry.id)];
					// Do Not Optimize!
					asm volatile("" : : "r,m"(n) : "memory");
					asm volatile("" : : "r,m"(entry.key) : "memory");

					switch (entry.type) {
					case BSS::Type::INSERT:
						n.set_key(entry.key);
						t.insert(n);
						break;
					case BSS::Type::ERASE:
						t.erase(entry.key);
						break;
					case BSS::Type::DELETE:
						t.remove(n);
						break;
					case BSS::Type::SEARCH: {
						auto it = t.find(entry.key);
						asm volatile("" : : "r,m"(it) : "memory");
					} break;
					case BSS::Type::LBOUND: {
						auto it = t.lower_bound(entry.key);
						asm volatile("" : : "r,m"(it) : "memory");
					} break;
					case BSS::Type::UBOUND: {
						auto it = t.upper_bound(entry.key);
						asm volatile("" : : "r,m"(it) : "memory");
					} break;
					default:
						break;
					}

					auto stopped_at = std::chrono::high_resolution_clock::now();
					auto duration = stopped_at - started_at;
					elapsed_round += static_cast<double>(
					    std::chrono::duration_cast<std::chrono::nanoseconds>(duration)
					        .count());
				}
			}

			if constexpr (depth_interval == 0) {
				this->out << this->benchmark_name << "," << this->name << ","
				          << this->repetitions << "," << elapsed_round << "\n";
			}

			this->repetitions--;
		}

		/* Compute statistics */ /*
		 double elapsed_sum = std::accumulate(
		     this->elapsed.begin(), this->elapsed.end(), 0.0, std::plus<double>());
		 double elapsed_avg =
		     elapsed_sum / static_cast<double>(this->elapsed.size());
		 auto var_builder = [&](double acc, double val) {
		   return acc + (val - elapsed_avg) * (val - elapsed_avg);
		 };
		 double elapsed_var =
		     std::accumulate(this->elapsed.begin(), this->elapsed.end(), 0.0,
		                     var_builder) /
		     static_cast<double>(this->elapsed.size());
		 double elapsed_stddev = sqrt(elapsed_var);

		 std::cout << " AVG TIME: " << elapsed_avg << " ns\n";
		 std::cout << " STD DEV: " << elapsed_stddev << " ns\n";*/
	}

	void
	create_nodes()
	{
		reader.reset();

		for (auto & buf = this->reader.get(CHUNKSIZE); buf.size() > 0;
		     this->reader.get(CHUNKSIZE)) {
			for (auto & entry : buf) {
				size_t id = reinterpret_cast<size_t>(entry.id);
				if (id + 1 > this->nodes.size()) {
					this->nodes.resize(id + 1);
				}
			}
		}
	}

	using BSS = ygg::utilities::BenchmarkSequenceStorage<KeyT>;
	BSS::Reader reader;

	std::string filename;
	std::string benchmark_name;
	std::string name;
	size_t repetitions;
	std::ostream & out;

	std::vector<Node> nodes;
};

template <class Options, template <class, class, class> class NodeBase,
          template <class, class, class, class, class> class TreeBase,
          class Traits>
class TreeBuilder {
public:
	class Node : public NodeBase<Node, Options, bool> {
	public:
		using ThisIsANodeTag_KeyType = unsigned int;

		unsigned int key;

		void
		set_key(unsigned int k)
		{
			this->key = k;
		}

		bool
		operator<(const Node & other) const
		{
			return this->key < other.key;
		}
		bool
		operator<(unsigned int rhs) const
		{
			return this->key < rhs;
		}
	};

	using Tree =
	    TreeBase<Node, Traits, Options, bool, ygg::utilities::flexible_less>;
};

template <class Node>
bool
operator<(typename Node::ThisIsANodeTag_KeyType lhs, const Node & rhs)
{
	return lhs < rhs.key;
}

int
main(int argc, char ** argv)
{
	if (argc < 4) {
		exit(-1);
	}

	size_t repetitions = 10;

	std::string output_prefix = argv[3];
	std::string output_timing = output_prefix + "_timing.csv";
	std::string output_depths = output_prefix + "_depths.csv";

	std::ofstream out_timing(output_timing);
	std::ofstream out_depths(output_depths);

	if (argc == 5) {
		repetitions = static_cast<size_t>(std::atoi(argv[4]));
	}

	using DefaultRBBuilder = TreeBuilder<ygg::DefaultOptions, ygg::RBTreeNodeBase,
	                                     ygg::RBTree, ygg::RBDefaultNodeTraits>;

	SequenceBenchmarker<DefaultRBBuilder::Tree, DefaultRBBuilder::Node, 0> rb_sb(
	    argv[1], argv[2], "DefaultRB", repetitions, out_timing);
	SequenceBenchmarker<DefaultRBBuilder::Tree, DefaultRBBuilder::Node, 500>
	    rb_sb_d(argv[1], argv[2], "DefaultRB", repetitions, out_depths);

	using DefaultWBBuilder = TreeBuilder<ygg::DefaultOptions, ygg::WBTreeNodeBase,
	                                     ygg::WBTree, ygg::WBDefaultNodeTraits>;

	SequenceBenchmarker<DefaultWBBuilder::Tree, DefaultWBBuilder::Node, 0>
	    def_wb_sb(argv[1], argv[2], "DefaultWB", repetitions, out_timing);
	SequenceBenchmarker<DefaultWBBuilder::Tree, DefaultWBBuilder::Node, 500>
	    def_wb_sb_d(argv[1], argv[2], "DefaultWB", repetitions, out_depths);

	using TDWBBuilder =
	    TreeBuilder<ygg::TreeOptions<ygg::TreeFlags::MULTIPLE,
	                                 ygg::TreeFlags::WBT_SINGLE_PASS>,
	                ygg::WBTreeNodeBase, ygg::WBTree, ygg::WBDefaultNodeTraits>;

	SequenceBenchmarker<TDWBBuilder::Tree, TDWBBuilder::Node, 0> td_wb_sb(
	    argv[1], argv[2], "TopDownWB", repetitions, out_timing);
	SequenceBenchmarker<TDWBBuilder::Tree, TDWBBuilder::Node, 500> td_wb_sb_d(
	    argv[1], argv[2], "TopDownWB", repetitions, out_depths);

	using WBTSinglepassBalTreeOptions =
	    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE,
	                     ygg::TreeFlags::WBT_SINGLE_PASS,
	                     ygg::TreeFlags::WBT_DELTA_NUMERATOR<5>,
	                     ygg::TreeFlags::WBT_DELTA_DENOMINATOR<2>,
	                     ygg::TreeFlags::WBT_GAMMA_NUMERATOR<2>,
	                     ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<1>>;

	using TDBalWBBuilder =
	    TreeBuilder<WBTSinglepassBalTreeOptions, ygg::WBTreeNodeBase, ygg::WBTree,
	                ygg::WBDefaultNodeTraits>;

	SequenceBenchmarker<TDBalWBBuilder::Tree, TDBalWBBuilder::Node, 0>
	    td_bal_wb_sb(argv[1], argv[2], "BalancedTopDownWB", repetitions,
	                 out_timing);
	SequenceBenchmarker<TDBalWBBuilder::Tree, TDBalWBBuilder::Node, 500>
	    td_bal_wb_sb_d(argv[1], argv[2], "BalancedTopDownWB", repetitions,
	                   out_depths);

	using WBTSinglepassLWTreeOptions =
	    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE,
	                     ygg::TreeFlags::WBT_SINGLE_PASS,
	                     ygg::TreeFlags::WBT_DELTA_NUMERATOR<3>,
	                     ygg::TreeFlags::WBT_DELTA_DENOMINATOR<1>,
	                     ygg::TreeFlags::WBT_GAMMA_NUMERATOR<4>,
	                     ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<3>>;

	using TDLWWBBuilder =
	    TreeBuilder<WBTSinglepassLWTreeOptions, ygg::WBTreeNodeBase, ygg::WBTree,
	                ygg::WBDefaultNodeTraits>;

	SequenceBenchmarker<TDLWWBBuilder::Tree, TDLWWBBuilder::Node, 0> td_lw_wb_sb(
	    argv[1], argv[2], "LWTopDownWB", repetitions, out_timing);
	SequenceBenchmarker<TDLWWBBuilder::Tree, TDLWWBBuilder::Node, 500>
	    td_lw_wb_sb_d(argv[1], argv[2], "LWTopDownWB", repetitions, out_depths);

	using WBTSinglepass32TreeOptions =
	    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE,
	                     ygg::TreeFlags::WBT_SINGLE_PASS,
	                     ygg::TreeFlags::WBT_DELTA_NUMERATOR<3>,
	                     ygg::TreeFlags::WBT_DELTA_DENOMINATOR<1>,
	                     ygg::TreeFlags::WBT_GAMMA_NUMERATOR<2>,
	                     ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<1>>;

	using TD32WBBuilder =
	    TreeBuilder<WBTSinglepass32TreeOptions, ygg::WBTreeNodeBase, ygg::WBTree,
	                ygg::WBDefaultNodeTraits>;

	SequenceBenchmarker<TD32WBBuilder::Tree, TD32WBBuilder::Node, 0> td_32_wb_sb(
	    argv[1], argv[2], "32TopDownWB", repetitions, out_timing);
	SequenceBenchmarker<TD32WBBuilder::Tree, TD32WBBuilder::Node, 500>
	    td_32_wb_sb_d(argv[1], argv[2], "32TopDownWB", repetitions, out_depths);

	using WBTSinglepassRelaxedTreeOptions =
	    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE,
	                     ygg::TreeFlags::WBT_SINGLE_PASS,
	                     ygg::TreeFlags::WBT_DELTA_NUMERATOR<4>,
	                     ygg::TreeFlags::WBT_DELTA_DENOMINATOR<1>,
	                     ygg::TreeFlags::WBT_GAMMA_NUMERATOR<4>,
	                     ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<3>>;

	using TDRelaxedWBBuilder =
	    TreeBuilder<WBTSinglepassRelaxedTreeOptions, ygg::WBTreeNodeBase,
	                ygg::WBTree, ygg::WBDefaultNodeTraits>;

	SequenceBenchmarker<TDRelaxedWBBuilder::Tree, TDRelaxedWBBuilder::Node, 0>
	    td_relaxed_wb_sb(argv[1], argv[2], "RelaxedTopDownWB", repetitions,
	                     out_timing);
	SequenceBenchmarker<TDRelaxedWBBuilder::Tree, TDRelaxedWBBuilder::Node, 500>
	    td_relaxed_wb_sb_d(argv[1], argv[2], "RelaxedTopDownWB", repetitions,
	                       out_depths);

	return 0;
}
