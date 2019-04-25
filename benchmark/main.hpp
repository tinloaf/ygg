#ifndef MAIN_HPP
#define MAIN_HPP

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <regex>
#include <string>
#include <utility>
#include <vector>

#include "common.hpp"

#define EXPERIMENT_SIZE 1000
#define BASE_SIZE 2048
#define DOUBLINGS 10

struct ConfigHolder
{
	int64_t experiment_size;
	int64_t base_size;
	size_t doublings;

	int64_t seed_start;
	size_t seed_count;
};

ConfigHolder CFG;

void
BuildRange(::benchmark::internal::Benchmark * b)
{
	for (int64_t seed = CFG.seed_start; seed < CFG.seed_start + (int64_t)CFG.seed_count;
	     ++seed) {
		for (size_t doubling = 0; doubling < CFG.doublings; ++doubling) {
			b->Args({CFG.base_size << doubling, CFG.experiment_size, seed});
		}
	}
}

int
main(int argc, char ** argv)
{
	int remaining_argc = argc;
	char ** remaining_argv = (char **)malloc((size_t)argc * sizeof(char *));

	CFG.experiment_size = EXPERIMENT_SIZE;
	CFG.base_size = BASE_SIZE;
	CFG.doublings = DOUBLINGS;
	CFG.seed_start = 4;
	CFG.seed_count = 2;
	
	std::string filter_re = "";

	remaining_argv[0] = argv[0];
	size_t j = 1;
	for (size_t i = 1; i < (size_t)argc; ++i) {
		if (strncmp(argv[i], "--papi", strlen("--papi")) == 0) {
			char * tok = strtok(argv[i + 1], ",");

			while (tok != NULL) {
				std::cout << tok << "\n";
				PAPI_MEASUREMENTS.emplace_back(tok);
				tok = strtok(NULL, ",");
			}

			i += 1;
			remaining_argc -= 2;
		} else if (strncmp(argv[i], "--doublings", strlen("--doublings")) == 0) {
			CFG.doublings = (size_t)atoi(argv[i + 1]);
			i += 1;
			remaining_argc -= 2;
		} else if (strncmp(argv[i], "--base_size", strlen("--base_size")) == 0) {
			CFG.base_size = (int64_t)atoi(argv[i + 1]);
			i += 1;
			remaining_argc -= 2;
		} else if (strncmp(argv[i], "--seed_start", strlen("--seed_start")) == 0) {
			CFG.seed_start = (int)atoi(argv[i + 1]);
			i += 1;
			remaining_argc -= 2;
		} else if (strncmp(argv[i], "--seed_count", strlen("--seed_count")) == 0) {
			CFG.seed_count = (size_t)atoi(argv[i + 1]);
			i += 1;
			remaining_argc -= 2;
		} else if (strncmp(argv[i], "--filter", strlen("--filter")) == 0) {
			std::cout << "Setting filter: " << argv[i + 1] << "\n" << std::flush;
			filter_re = std::string(argv[i + 1]);
			i += 1;
			remaining_argc -= 2;
		} else if (strncmp(argv[i], "--experiment_size",
		                   strlen("--experiment_size")) == 0) {
			CFG.experiment_size = (int64_t)atoi(argv[i + 1]);
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

	boost::hana::for_each(plugins, [&](auto plugin_cls) {
		using plugin = typename decltype(plugin_cls)::type;

		auto name = plugin::get_name();

		bool skip = false;
		if (filter_re.compare("") != 0) {
			std::regex re(filter_re, std::regex_constants::ECMAScript |
			                             std::regex_constants::icase);
			if (!std::regex_match(name, re)) {
				skip = true;
			}
		}

		if (!skip) {
			auto * bench = new plugin();
			bench->set_name(name);
			/*
			RangeVector ranges;
			ranges.emplace_back(base_size, base_size * (1 << doublings));
			ranges.emplace_back(experiment_size, experiment_size);
			bench->RangeMultiplier(2);
			bench->Ranges(ranges);
			*/
			bench->Apply(BuildRange);
			::benchmark::internal::RegisterBenchmarkInternal(bench);
		}
	});

	PAPI_STATS_WRITTEN = false;

	::benchmark::Initialize(&remaining_argc, remaining_argv);
	if (::benchmark::ReportUnrecognizedArguments(remaining_argc,
	                                             remaining_argv)) {
		return 1;
	}

	::benchmark::RunSpecifiedBenchmarks();
	return 0;
}

#endif
