#include "../src/benchmark_sequence.hpp"
#include "../src/options.hpp"
#include "../src/ygg.hpp"

using namespace ygg;

#include <chrono>
#include <numeric>
#include <ostream>
#include <type_traits>
#include <unordered_map>

template <class Tree, class Node>
class SequenceBenchmarker {
private:
	constexpr static size_t CHUNKSIZE = 10000;
	using KeyT = unsigned int;

public:
	SequenceBenchmarker(std::string filename_in, std::string benchmark_name_in,
	                    std::string name_in, size_t repetitions_in)
	    : reader(filename_in.c_str()), filename(filename_in),
	      benchmark_name(benchmark_name_in), name(name_in),
	      repetitions(repetitions_in)
	{
		this->create_nodes();
		this->run();
	};

private:
	void
	run()
	{

		while (this->repetitions >= 1) {
			double elapsed_round = 0;
			this->reader.reset();
			Tree t;

			std::cerr << this->repetitions << " iterations remaining...\n";

			for (auto & buf = this->reader.get(CHUNKSIZE); buf.size() > 0;
			     this->reader.get(CHUNKSIZE)) {
				auto started_at = std::chrono::high_resolution_clock::now();

				for (auto & entry : buf) {
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

			std::cout << this->benchmark_name << "," << this->name << ","
			          << this->repetitions << "," << elapsed_round << "\n";

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
	if (argc < 3) {
		exit(-1);
	}

	size_t repetitions = 10;

	if (argc == 4) {
		repetitions = static_cast<size_t>(std::atoi(argv[3]));
	}

	using DefaultRBBuilder = TreeBuilder<ygg::DefaultOptions, ygg::RBTreeNodeBase,
	                                     ygg::RBTree, ygg::RBDefaultNodeTraits>;

	SequenceBenchmarker<DefaultRBBuilder::Tree, DefaultRBBuilder::Node> rb_sb(
	    argv[1], argv[2], "DefaultRB", repetitions);

	using DefaultWBBuilder = TreeBuilder<ygg::DefaultOptions, ygg::WBTreeNodeBase,
	                                     ygg::WBTree, ygg::WBDefaultNodeTraits>;

	SequenceBenchmarker<DefaultWBBuilder::Tree, DefaultWBBuilder::Node> def_wb_sb(
	    argv[1], argv[2], "DefaultWB", repetitions);

	using TDWBBuilder =
	    TreeBuilder<ygg::TreeOptions<ygg::TreeFlags::MULTIPLE,
	                                 ygg::TreeFlags::WBT_SINGLE_PASS>,
	                ygg::WBTreeNodeBase, ygg::WBTree, ygg::WBDefaultNodeTraits>;

	SequenceBenchmarker<TDWBBuilder::Tree, TDWBBuilder::Node> td_wb_sb(
	    argv[1], argv[2], "TopDownWB", repetitions);

	using WBTSinglepassBalTreeOptions =
	    ygg::TreeOptions<ygg::TreeFlags::MULTIPLE,
	                     ygg::TreeFlags::WBT_SINGLE_PASS,
	                     ygg::TreeFlags::WBT_DELTA_NUMERATOR<5>,
	                     ygg::TreeFlags::WBT_DELTA_DENOMINATOR<2>,
	                     ygg::TreeFlags::WBT_GAMMA_NUMERATOR<2>,
	                     ygg::TreeFlags::WBT_GAMMA_DENOMINATOR<1>>;

	using TDBalWBBuilder =
	    TreeBuilder<WBTSinglepassBalTreeOptions,

	                ygg::WBTreeNodeBase, ygg::WBTree, ygg::WBDefaultNodeTraits>;

	SequenceBenchmarker<TDBalWBBuilder::Tree, TDBalWBBuilder::Node> td_bal_wb_sb(
	    argv[1], argv[2], "BalancedTopDownWB", repetitions);

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

	SequenceBenchmarker<TDLWWBBuilder::Tree, TDLWWBBuilder::Node> td_lw_wb_sb(
	    argv[1], argv[2], "LWTopDownWB", repetitions);

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

	SequenceBenchmarker<TD32WBBuilder::Tree, TD32WBBuilder::Node> td_32_wb_sb(
	    argv[1], argv[2], "32TopDownWB", repetitions);

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

	SequenceBenchmarker<TDRelaxedWBBuilder::Tree, TDRelaxedWBBuilder::Node>
	    td_relaxed_wb_sb(argv[1], argv[2], "RelaxedTopDownWB", repetitions);

	return 0;
}
