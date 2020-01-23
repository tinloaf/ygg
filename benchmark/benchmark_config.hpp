#ifndef BENCHMARK_CONFIG_HPP
#define BENCHMARK_CONFIG_HPP

struct ConfigHolder
{
	int64_t experiment_size;
	double relative_experiment_size;
	bool use_relative_size = false;
	int64_t base_size;
	size_t doublings;

	int64_t seed_start;
	size_t seed_count;

	int64_t iteration_count;

	size_t ziptree_export_ranks = 0;
};

ConfigHolder CFG;

#endif
