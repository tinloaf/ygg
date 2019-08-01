#include "random.hpp"

#include <cassert>
#include <random>

Randomizer::Randomizer(unsigned long seed) : rng(seed) {}

ZipfDistr::ZipfDistr(unsigned long seed, double s_in)
    : Randomizer(seed), s(s_in), cdf_dist(0, 1)
{}

double
ZipfDistr::ghn(size_t n, double m)
{
	static GhnCache c;

	if (c.find({n, m}) != c.end()) {
		return c.find({n, m})->second;
	}

	double val = 0;
	for (size_t i = 1; i <= n; i++) {
		val += 1 / (std::pow(i, m));
	}

	c[{n, m}] = val;

	return val;
}

size_t
ZipfDistr::inverse_ghn_on_n(double ghn, double m)
{
	static InverseGhnCache c;

	if (c.find({ghn, m}) != c.end()) {
		return c.find({ghn, m})->second;
	}

	double val = 0;
	for (size_t i = 1;; i++) {
		val += 1 / (std::pow(i, m));
		if (val >= ghn) {
			c[{ghn, m}] = i;
			return i;
		}
	}
}

int
ZipfDistr::generate(int min, int max)
{
	/* the CDF for a value of k is ghn(k,s) / ghn( (max - min),s). We draw
	   uniformly at random from [0,1) and set that as the cumulative density:

	      ghn(k,s) = cdf * ghn( (max-min), s)
	   => k = inverse_ghn(cdf * ghn((max-min), s), s)

*/
	double cdf = this->cdf_dist(this->rng);

	assert(max > min);
	auto val = this->inverse_ghn_on_n(
	    cdf * this->ghn(static_cast<size_t>((max - min)), this->s), this->s);
	assert(val < static_cast<unsigned long>(max));

	return static_cast<int>(val);
}

UniformDistr::UniformDistr(unsigned long seed) : Randomizer(seed) {}

int
UniformDistr::generate(int min, int max)
{
	std::uniform_int_distribution<int> distr(min, max - 1);
	return distr(this->rng);
}

MaekinenSkewedDistr::MaekinenSkewedDistr(unsigned long seed, size_t n_in,
                                         size_t change_freq_in,
                                         size_t partition_count_in,
                                         double partition_size_in)
    : Randomizer(seed), n(n_in), change_freq(change_freq_in),
      partition_count(partition_count_in), partition_size(partition_size_in),
      counter(0)
{}

int
MaekinenSkewedDistr::generate(int min, int max)
{
	this->counter++;

	if (this->counter % this->n == 0) {
		std::uniform_int_distribution<int> distr(min, max - 1);
		return distr(this->rng);
	} else {
		size_t partition_size_absolute =
		    static_cast<size_t>(std::round((max - min) * this->partition_size));
		size_t partition_no =
		    static_cast<size_t>(this->counter / this->change_freq) %
		    this->partition_count;
		if (partition_no % 2 == 0) {
			int left_boundary =
			    static_cast<int>((1 + partition_no) * partition_size_absolute);
			int right_boundary = static_cast<int>(static_cast<size_t>(left_boundary) +
			                                      partition_size_absolute);
			assert(left_boundary < right_boundary);
			std::uniform_int_distribution<int> distr(left_boundary,
			                                         right_boundary - 1);
			return distr(this->rng);
		} else {
			int right_boundary = static_cast<int>(
			    static_cast<size_t>(max) - (partition_no * partition_size_absolute));
			int left_boundary = static_cast<int>(static_cast<size_t>(right_boundary) -
			                                     partition_size_absolute);
			assert(left_boundary < right_boundary);
			std::uniform_int_distribution<int> distr(left_boundary,
			                                         right_boundary - 1);
			return distr(this->rng);
		}
	}
}
