#include "random.hpp"

#include <random>
#include <cassert>

Randomizer::Randomizer(unsigned long seed) : rng(seed) {}

ZipfDistr::ZipfDistr(unsigned long seed, double s_in)
    : Randomizer(seed), s(s_in), cdf_dist(0, 1), ghn_cache(),
      inverse_ghn_cache()
{}

double
ZipfDistr::ghn(size_t n, double m)
{
	if (this->ghn_cache.find({n, m}) != this->ghn_cache.end()) {
		return this->ghn_cache.find({n, m})->second;
	}

	double val = 0;
	for (size_t i = 1; i <= n; i++) {
		val += 1 / (std::pow(i, m));
	}

	this->ghn_cache[{n, m}] = val;

	return val;
}

size_t
ZipfDistr::inverse_ghn_on_n(double ghn, double m)
{
	if (this->inverse_ghn_cache.find({ghn, m}) != this->inverse_ghn_cache.end()) {
		return this->inverse_ghn_cache.find({ghn, m})->second;
	}

	double val = 0;
	for (size_t i = 1;; i++) {
		val += 1 / (std::pow(i, m));
		if (val >= ghn) {
			this->inverse_ghn_cache[{ghn, m}] = i;
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
	assert(val < static_cast<unsigned long>(max)m);
	
	return static_cast<int>(val);
}

UniformDistr::UniformDistr(unsigned long seed) : Randomizer(seed) {}

int
UniformDistr::generate(int min, int max)
{
	std::uniform_int_distribution<int> distr(min, max - 1);
	return distr(this->rng);
}
