#ifndef BENCH_COMMON_HPP
#define BENCH_COMMON_HPP

#include "benchmark.h"
#include <algorithm>
#include <boost/intrusive/set.hpp>
#include <draup.hpp>
#include <random>
#include <vector>

#ifdef USEPAPI
#include <papi.h>
#endif

// Draup Registration
#define REGISTER(BaseClass, Method)                                            \
	DRAUP_REGISTER(BaseClass##_##Method##_Benchmark)

// Experiment Names
constexpr auto delete_experiment_c = BOOST_HANA_STRING("Delete");
using DeleteExperiment = decltype(delete_experiment_c);
constexpr auto erase_experiment_c = BOOST_HANA_STRING("Erase");
using EraseExperiment = decltype(erase_experiment_c);
constexpr auto move_experiment_c = BOOST_HANA_STRING("Move");
using MoveExperiment = decltype(move_experiment_c);
constexpr auto insert_experiment_c = BOOST_HANA_STRING("Insert");
using InsertExperiment = decltype(insert_experiment_c);
constexpr auto search_experiment_c = BOOST_HANA_STRING("Search");
using SearchExperiment = decltype(search_experiment_c);

std::vector<std::string> PAPI_MEASUREMENTS;
bool PAPI_STATS_WRITTEN;

class PapiMeasurements {
public:
	void
	initialize()
	{
#ifdef USEPAPI
		if (!this->initialized) {
			this->initialized = true;

			int num_counters = PAPI_num_counters();
			if (!PAPI_STATS_WRITTEN) {
				std::cout << "## Your system has " << num_counters
				          << " PAPI counters.\n";
			}

			if (num_counters == 0) {
				std::cout
				    << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
				std::cout
				    << "!!        Warning: No PAPI counters available!       !!\n";
				std::cout << "!! Use the avail and component_avail tools !!\n";
				std::cout
				    << "!! to figure out why. If you are on Linux, you most  !!\n";
				std::cout
				    << "!! probably need to set                              !!\n";
				std::cout
				    << "!! /proc/sys/kernel/perf_event_paranoid to 0         !!\n";
				std::cout
				    << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
			}

			int event_code;
			event_code = 0 | PAPI_NATIVE_MASK;

			this->selected_events.clear();
			for (const std::string & event_str : PAPI_MEASUREMENTS) {
				if (PAPI_event_name_to_code(event_str.c_str(), &event_code) !=
				    PAPI_OK) {
					std::cerr << "PAPI event " << event_str << " not found!\n";
					exit(-1);
				}

				this->selected_events.push_back(event_code);
				if (!PAPI_STATS_WRITTEN) {
					std::cout << "## Registering PAPI event " << event_str << " (ID "
					          << event_code << ")\n";
				}
			}
			PAPI_STATS_WRITTEN = true;
		}

		this->event_counts.resize(this->selected_events.size());
		this->event_count_accu.resize(this->selected_events.size());
		std::fill(this->event_count_accu.begin(), this->event_count_accu.end(), 0);
#endif
	}

	void
	start()
	{
#ifdef USEPAPI
		// TODO error handling
		PAPI_start_counters(this->selected_events.data(),
		                    (int)this->selected_events.size());
#endif
	}

	void
	stop()
	{
#ifdef USEPAPI
		// TODO error handling
		PAPI_stop_counters(this->event_counts.data(),
		                   (int)this->event_counts.size());
		std::transform(this->event_counts.begin(), this->event_counts.end(),
		               this->event_count_accu.begin(),
		               this->event_count_accu.begin(), std::plus<long long>());
#endif
	}

	void
	report_and_reset(::benchmark::State & state)
	{
#ifdef USEPAPI
		for (size_t i = 0; i < PAPI_MEASUREMENTS.size(); ++i) {
			state.counters[PAPI_MEASUREMENTS[i]] = (double)this->event_count_accu[i];
		}
		std::fill(this->event_count_accu.begin(), this->event_count_accu.end(), 0);
#else
		(void)state;
#endif
	}

private:
#ifdef USEPAPI
	std::vector<int> selected_events;
	std::vector<long long> event_counts;
	std::vector<long long> event_count_accu;
	bool initialized = false;
#endif
};

#endif
