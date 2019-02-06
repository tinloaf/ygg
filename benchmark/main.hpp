#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>

#ifdef USEPAPI
#include <papi.h>
#endif

std::vector<std::string> PAPI_MEASUREMENTS;
bool PAPI_STATS_WRITTEN;

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

	PAPI_STATS_WRITTEN = false;
	
	std::cout << "Found " << PAPI_MEASUREMENTS.size() << "events.\n" << std::flush;
	
	::benchmark::Initialize(&remaining_argc, remaining_argv);
	if (::benchmark::ReportUnrecognizedArguments(remaining_argc, remaining_argv)) {
		return 1;
	}
	::benchmark::RunSpecifiedBenchmarks();
	return 0;
}
