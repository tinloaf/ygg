#ifndef YGG_PAIRED_HPP
#define YGG_PAIRED_HPP

#include "common_bst.hpp"

#include <benchmark.h>
#include <boost/math/distributions/binomial.hpp>
#include <chrono>
#include <cmath>
#include <iterator>
#include <numeric>

template <class InterfaceA, class InterfaceB, class Experiment, class Options>
class PairedTester {
public:
	struct Result
	{
		double p_value;
		double cohens_d;
	};

	template <class Executor>
	Result
	run(size_t iterations, size_t inner_iterations, size_t base_size,
	    size_t experiment_size, int initial_seed)
	{
		int seed = initial_seed;

		for (size_t i = 0; i < iterations; ++i) {
			A.initialize(base_size, experiment_size, seed);
			B.initialize(base_size, experiment_size, seed);

			double valueA;
			double valueB;
			// Mitigate order effects
			if (i % 2 == 0) {
				valueA = A.template run<Executor>(inner_iterations);
				valueB = B.template run<Executor>(inner_iterations);
			} else {
				valueB = B.template run<Executor>(inner_iterations);
				valueA = A.template run<Executor>(inner_iterations);
			}
			this->results.push_back({valueA, valueB});

			seed++;
		}

		std::vector<double> deltas;
		std::transform(this->results.begin(), this->results.end(),
		               std::back_inserter(deltas),
		               [](auto & p) { return p.first - p.second; });

		// Binomial Test
		size_t A_better_count = std::count_if(deltas.begin(), deltas.end(),
		                                      [](auto & d) { return d < 0; });
		double p_val = boost::math::pdf(
		    boost::math::binomial_distribution<double>(deltas.size(), 0.5),
		    A_better_count);

		// Cohen's D
		double delta_mean = std::accumulate(deltas.begin(), deltas.end(), 0.0) /
		                    static_cast<double>(deltas.size());
		double mean_a =
		    std::accumulate(this->results.begin(), this->results.end(), 0.0,
		                    [](double val, auto & p) { return val + p.first; });
		double mean_b =
		    std::accumulate(this->results.begin(), this->results.end(), 0.0,
		                    [](double val, auto & p) { return val + p.second; });
		double variance_a = std::accumulate(
		    this->results.begin(), this->results.end(), 0.0,
		    [&](double val, auto & p) {
			    return val + ((p.first - mean_a) * (p.first - mean_a));
		    });
		double variance_b = std::accumulate(
		    this->results.begin(), this->results.end(), 0.0,
		    [&](double val, auto & p) {
			    return val + ((p.second - mean_b) * (p.second - mean_b));
		    });
		variance_a /= static_cast<double>(this->results.size() - 1);
		variance_b /= static_cast<double>(this->results.size() - 1);
		double pooled_stddev =
		    std::sqrt(((this->results.size() - 1) * (variance_a + variance_b)) /
		              (2 * this->results.size() - 2));
		double cohen_d = delta_mean / pooled_stddev;

		return {p_val, cohen_d};
	}

private:
	template <class Base>
	class Wrapper : public Base {
	public:
		virtual void BenchmarkCase(
		    ::benchmark::State &){}; // Artiface from re-using GBenchmark code

		template <class Executor>
		double
		run(size_t inner_iterations) noexcept
		{
			double duration_sum = 0;
			for (size_t i = 0; i < inner_iterations; ++i) {
				// Clobber memory
				asm volatile("" : : : "memory");
				auto started_at = std::chrono::high_resolution_clock::now();
				Executor::run(*this);
				auto finished_at = std::chrono::high_resolution_clock::now();
				// Pretend we need the tree's data as input.
				asm volatile(""
				             :
				             : "g"(&this->t), "g"(this->fixed_nodes.data()),
				               "g"(this->experiment_nodes.data()),
				               "g"(this->experiment_node_pointers.data())
				             : "memory");

				auto duration = finished_at - started_at;
				duration_sum += static_cast<double>(
				    std::chrono::duration_cast<std::chrono::nanoseconds>(duration)
				        .count());
				Executor::revert(*this);
			}
			return duration_sum;
		}
	};

	Wrapper<BSTFixture<InterfaceA, Experiment, Options>> A;
	Wrapper<BSTFixture<InterfaceB, Experiment, Options>> B;

	std::vector<std::pair<double, double>> results;
};

// Hacky hack. Define functions that the reuse of the google benchmark fixture
// pulls in.
namespace benchmark {
namespace internal {

int
InitializeStreams()
{
	return 0;
};

Benchmark::Benchmark(char const *) {}
Benchmark::~Benchmark() {}

}; // namespace internal
}; // namespace benchmark

#endif
