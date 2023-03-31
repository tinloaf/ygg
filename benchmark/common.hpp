#ifndef BENCH_COMMON_HPP
#define BENCH_COMMON_HPP

#include "benchmark/benchmark.h"
#include "benchmark_config.hpp"

#include <algorithm>
#include <boost/intrusive/set.hpp>
#include <chrono>
#include <draup.hpp>
#include <iostream>
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

class Clock {
public:
	void
	start()
	{
		this->started = std::chrono::steady_clock::now();
	};

	double
	get()
	{
		auto stopped = std::chrono::steady_clock::now();
		long int ret = std::chrono::duration_cast<std::chrono::nanoseconds>(
		                   stopped - this->started)
		                   .count();
		return static_cast<double>(ret) / 1000000000.0;
	};

private:
	std::chrono::time_point<std::chrono::steady_clock> started;
};

class PointerCountCallback {
public:
	static void
	start()
	{
		PointerCountCallback::running = true;
	}
	static void
	stop()
	{
		PointerCountCallback::running = false;
	}

	static void
	get_left()
	{
		if (PointerCountCallback::running)
			PointerCountCallback::get_left_c++;
	}
	static void
	get_right()
	{
		if (PointerCountCallback::running)
			PointerCountCallback::get_right_c++;
	}
	static void
	get_parent()
	{
		if (PointerCountCallback::running)
			PointerCountCallback::get_parent_c++;
	}
	static void
	set_left()
	{
		if (PointerCountCallback::running)
			PointerCountCallback::set_left_c++;
	}
	static void
	set_right()
	{
		if (PointerCountCallback::running)
			PointerCountCallback::set_right_c++;
	}
	static void
	set_parent()
	{
		if (PointerCountCallback::running)
			PointerCountCallback::set_parent_c++;
	}

	static void
	reset()
	{
		PointerCountCallback::get_left_c = 0;
		PointerCountCallback::get_right_c = 0;
		PointerCountCallback::get_parent_c = 0;
		PointerCountCallback::set_left_c = 0;
		PointerCountCallback::set_right_c = 0;
		PointerCountCallback::set_parent_c = 0;
		PointerCountCallback::running = false;
	}

	static void
	report(::benchmark::State & state)
	{
#ifdef COUNTOPS
		state.counters["GET_RIGHT"] = ::benchmark::Counter(
		    static_cast<double>(PointerCountCallback::get_right_c),
		    ::benchmark::Counter::Flags::kAvgIterations);
		state.counters["GET_LEFT"] = ::benchmark::Counter(
		    static_cast<double>(PointerCountCallback::get_left_c),
		    ::benchmark::Counter::Flags::kAvgIterations);
		state.counters["GET_PARENT"] = ::benchmark::Counter(
		    static_cast<double>(PointerCountCallback::get_parent_c),
		    ::benchmark::Counter::Flags::kAvgIterations);
		state.counters["GET_TOTAL"] = ::benchmark::Counter(
		    static_cast<double>(PointerCountCallback::get_parent_c +
		                        PointerCountCallback::get_left_c +
		                        PointerCountCallback::get_right_c),
		    ::benchmark::Counter::Flags::kAvgIterations);

		state.counters["SET_RIGHT"] = ::benchmark::Counter(
		    static_cast<double>(PointerCountCallback::set_right_c),
		    ::benchmark::Counter::Flags::kAvgIterations);
		state.counters["SET_LEFT"] = ::benchmark::Counter(
		    static_cast<double>(PointerCountCallback::set_left_c),
		    ::benchmark::Counter::Flags::kAvgIterations);
		state.counters["SET_PARENT"] = ::benchmark::Counter(
		    static_cast<double>(PointerCountCallback::set_parent_c),
		    ::benchmark::Counter::Flags::kAvgIterations);
		state.counters["SET_TOTAL"] = ::benchmark::Counter(
		    static_cast<double>(PointerCountCallback::set_parent_c +
		                        PointerCountCallback::set_left_c +
		                        PointerCountCallback::set_right_c),
		    ::benchmark::Counter::Flags::kAvgIterations);
#else
		(void)state;
#endif
	}

private:
	static bool running;
	static size_t get_left_c;
	static size_t get_right_c;
	static size_t get_parent_c;
	static size_t set_left_c;
	static size_t set_right_c;
	static size_t set_parent_c;
};

bool PointerCountCallback::running;
size_t PointerCountCallback::get_left_c;
size_t PointerCountCallback::get_right_c;
size_t PointerCountCallback::get_parent_c;
size_t PointerCountCallback::set_left_c;
size_t PointerCountCallback::set_right_c;
size_t PointerCountCallback::set_parent_c;

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
		                    static_cast<int>(this->selected_events.size()));
#endif
	}

	void
	stop()
	{
#ifdef USEPAPI
		// TODO error handling
		PAPI_stop_counters(this->event_counts.data(),
		                   static_cast<int>(this->event_counts.size()));
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
			state.counters[PAPI_MEASUREMENTS[i]] =
			    ::benchmark::Counter(static_cast<double>(this->event_count_accu[i]),
			                         ::benchmark::Counter::Flags::kAvgIterations);
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
