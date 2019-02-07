#ifndef MAIN_HPP
#define MAIN_HPP

#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <iostream>
#include <utility>

#include "common.hpp"

#define EXPERIMENT_SIZE 1024
#define BASE_SIZE 2048
#define DOUBLINGS 10

int main(int argc, char ** argv)
{
	int remaining_argc = argc;
	char ** remaining_argv = (char **)malloc((size_t)argc * sizeof(char *));

	int64_t experiment_size = EXPERIMENT_SIZE;
	int64_t base_size = BASE_SIZE;
	int64_t doublings = DOUBLINGS;
	
	remaining_argv[0] = argv[0];
	size_t j = 1;
	for (size_t i = 1 ; i < (size_t)argc; ++i) {
		if (strncmp(argv[i], "--papi", strlen("--papi")) == 0) {
			char * tok = strtok(argv[i+1], ",");

			while (tok != NULL) {
				std::cout << tok << "\n";
				PAPI_MEASUREMENTS.emplace_back(tok);
				tok = strtok(NULL, ",");
			}
			
			i += 1;
			remaining_argc -= 2;
		} else if (strncmp(argv[i], "--doublings", strlen("--doublings")) == 0) {
			doublings = (size_t)atoi(argv[i+1]);
			i += 1;
			remaining_argc -= 2;
		} else if (strncmp(argv[i], "--base_size", strlen("--base_size")) == 0) {
			base_size = (size_t)atoi(argv[i+1]);
			i += 1;
			remaining_argc -= 2;
		} else if (strncmp(argv[i], "--experiment_size", strlen("--experiment_size")) == 0) {
			experiment_size = (size_t)atoi(argv[i+1]);
			i += 1;
			remaining_argc -= 2;
		} else {
			remaining_argv[j++] = argv[i];
		}
	}


	/* 
	 * Register all tests
	 */
	auto plugins = DRAUP_GET_REGISTERED();
	using RangeVector = std::vector<std::pair<int64_t, int64_t>>;
	
	DRAUP_FOR_EACH(plugins, {
			auto * bench = new plugin();
			RangeVector ranges;
			ranges.emplace_back(base_size, base_size * (1 << doublings));
			ranges.emplace_back(experiment_size, experiment_size);
			bench->RangeMultiplier(2);
			bench->Ranges(ranges);
			::benchmark::internal::RegisterBenchmarkInternal(bench);});
	
	PAPI_STATS_WRITTEN = false;
	
	::benchmark::Initialize(&remaining_argc, remaining_argv);
	if (::benchmark::ReportUnrecognizedArguments(remaining_argc, remaining_argv)) {
		return 1;
	}

	::benchmark::RunSpecifiedBenchmarks();
	return 0;
}

#endif
