#include "random.hpp"

#include <random>

Randomizer::Randomizer(unsigned long seed) : rng(seed) {}

ZipfDistr::ZipfDistr(unsigned long seed, double s_in)
    : Randomizer(seed), s(s_in), cdf_dist(0, 1)
{}

double
ZipfDistr::ghn(size_t n, double m)
{
	double val = 0;
	for (size_t i = 1; i <= n; i++) {
		val += 1 / (std::pow(i, m));
	}
	return val;
}

size_t
ZipfDistr::inverse_ghn_on_n(double ghn, double m)
{
	double val = 0;
	for (size_t i = 1;; i++) {
		val += 1 / (std::pow(i, m));
		if (val >= ghn) {
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

	auto val =
	    this->inverse_ghn_on_n(cdf * this->ghn((max - min), this->s), this->s);

	return static_cast<int>(val);
}

UniformDistr::UniformDistr(unsigned long seed) : Randomizer(seed) {}

int
UniformDistr::generate(int min, int max)
{
	std::uniform_int_distribution<int> distr(min, max);
	return distr(this->rng);
}
