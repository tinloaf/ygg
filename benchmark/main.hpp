#ifndef MAIN_HPP
#define MAIN_HPP

#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <iostream>

#include "common.hpp"

#define EXPERIMENT_SIZE 1024
#define BASE_SIZE 2048
#define DOUBLINGS 10

int main(int argc, char ** argv)
{
	int remaining_argc = argc;
	char ** remaining_argv = (char **)malloc((size_t)argc * sizeof(char *));
	
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
		} else {
			remaining_argv[j++] = argv[i];
		}
	}


	/* 
	 * Register all tests
	 */
	auto plugins = DRAUP_GET_REGISTERED();
	DRAUP_FOR_EACH(plugins, {
			auto * bench = new plugin();
			bench->RangeMultiplier(2);
			bench->Ranges({{BASE_SIZE, BASE_SIZE * (1 << DOUBLINGS)}, {EXPERIMENT_SIZE, EXPERIMENT_SIZE}});
			::benchmark::internal::RegisterBenchmarkInternal(bench);
	});
	
	PAPI_STATS_WRITTEN = false;
	
	::benchmark::Initialize(&remaining_argc, remaining_argv);
	if (::benchmark::ReportUnrecognizedArguments(remaining_argc, remaining_argv)) {
		return 1;
	}

	::benchmark::RunSpecifiedBenchmarks();
	return 0;
}

#endif
