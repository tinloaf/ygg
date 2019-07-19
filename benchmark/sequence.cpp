#include "../src/benchmark_sequence.hpp"
#include "../src/ygg.hpp"
#include "../src/options.hpp"

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
	using KeyT = int;

public:
	SequenceBenchmarker(std::string filename_in)
	    : reader(filename_in.c_str()), filename(filename_in), repetitions(100000),
	      elapsed(0.0)
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
			this->elapsed.push_back(elapsed_round);
			this->repetitions--;
		}

		/* Compute statistics */
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
		std::cout << " STD DEV: " << elapsed_stddev << " ns\n";
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
	size_t repetitions;

	std::vector<double> elapsed;

	std::vector<Node> nodes;
};

using MyTreeOptions = DefaultOptions;

class Node : public RBTreeNodeBase<Node, MyTreeOptions> {
public:
	int key;

	void
	set_key(int k)
	{
		this->key = k;
	}

	bool
	operator<(const Node & other) const
	{
		return this->key < other.key;
	}
};

bool
operator<(const Node & lhs, int rhs)
{
	return lhs.key < rhs;
}
bool
operator<(int lhs, const Node & rhs)
{
	return lhs < rhs.key;
}

using MyTree = RBTree<Node, RBDefaultNodeTraits, MyTreeOptions>;

int
main(int argc, char ** argv)
{
	if (argc != 2) {
		exit(-1);
	}

	SequenceBenchmarker<MyTree, Node> sb(argv[1]);

	return 0;
}
