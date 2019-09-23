#include "random.hpp"

#include <cassert>
#include <cmath>
#include <map>
#include <random>

Randomizer::Randomizer(unsigned long seed) : rng(seed), stored_seed(seed) {}

unsigned long
Randomizer::get_seed() const noexcept
{
	return this->stored_seed;
}

const char *
ZipfDistr::get_name() const noexcept
{
	return "Zipf";
}

const char *
UniformDistr::get_name() const noexcept
{
	return "Uniform";
}

const char *
MaekinenSkewedDistr::get_name() const noexcept
{
	return "Skewed";
}

int
ZipfDistr::get_default_max() const noexcept
{
	return 1000000; // TODO parameterize?
}

int
UniformDistr::get_default_max() const noexcept
{
	return std::numeric_limits<int>::max();
}

int
MaekinenSkewedDistr::get_default_max() const noexcept
{
	return std::numeric_limits<int>::max();
}

ZipfDistr::ZipfSampler &
ZipfDistr::get_sampler(int min, int max)
{
	static std::map<std::pair<size_t, double>, ZipfSampler> samplers;

	std::pair<size_t, double> key{max - min, this->exponent};
	if (samplers.find(key) == samplers.end()) {
		samplers.insert(
		    {key, ZipfSampler(this->exponent, static_cast<size_t>(max - min),
		                      this->rng)});
	}

	return samplers.at(key);
}

ZipfDistr::ZipfDistr(unsigned long seed, double exponent_in)
    : Randomizer(seed), exponent(exponent_in)
{}

ZipfDistr::ZipfSampler::ZipfSampler(double exponent_in,
                                    size_t number_of_elements_in,
                                    std::mt19937 & rng_in)
    : exponent(exponent_in), number_of_elements(number_of_elements_in),
      rng(rng_in), h_integral_x1(h_integral(1.5) - 1.0),
      h_integral_number_of_elements(static_cast<double>(number_of_elements_in) +
                                    0.5),
      s(2.0 - h_integral_inverse(h_integral(2.5) - h(2.0)))
{}

double
ZipfDistr::ZipfSampler::h_integral(double x) const noexcept
{
	double log_x = std::log(x);
	return this->helper2((1.0 - this->exponent) * log_x) * log_x;
}

double
ZipfDistr::ZipfSampler::h(double x) const noexcept
{
	return std::exp(-1 * this->exponent * std::log(x));
}

double
ZipfDistr::ZipfSampler::h_integral_inverse(double x) const noexcept
{
	double t = x * (1.0 - this->exponent);
	if (t < -1.0) { // Numerical issues
		t = -1.0;
	}
	return std::exp(this->helper1(t) * x);
}

double
ZipfDistr::ZipfSampler::helper1(double x) const noexcept
{
	// Originial implementation uses a tailor expansion for small x
	if (std::abs(x) > 1e-8) {
		return std::log1p(x) / x;
	} else {
		return 1.0 - x * ((1.0 / 2.0) - x * ((1.0 / 3.0) - x * (1.0 / 4.0)));
	}
}

double
ZipfDistr::ZipfSampler::helper2(double x) const noexcept
{
	// Use tailor expansion for small x
	if (std::abs(x) > 1e-8) {
		return std::expm1(x) / x;
	} else {
		return 1.0 +
		       x * (1.0 / 2.0) * (1.0 + x * (1.0 / 3.0) * (1.0 + x * (1.0 / 4.0)));
	}
}

int
ZipfDistr::ZipfSampler::generate()
{
	std::uniform_real_distribution<double> udistr(0.0, 1.0);
	while (true) {
		double u =
		    this->h_integral_number_of_elements +
		    udistr(this->rng) * (h_integral_x1 - h_integral_number_of_elements);
		double x = h_integral_inverse(u);
		int k = static_cast<int>(x + 0.5);

		// Fix numerical inaccuracies
		if (k < 1) {
			k = 1;
		} else if (static_cast<size_t>(k) > this->number_of_elements) {
			k = static_cast<int>(
			    this->number_of_elements); // TODO assert that they are not too many
		}

		// Accept k based on the right probabilities
		if (k - x <= s || u >= h_integral(k + 0.5) - h(k)) {
			return k;
		}
	}
}

int
ZipfDistr::generate(int min, int max)
{
	auto sampler = this->get_sampler(min, max);
	return sampler.generate();
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
		size_t partition_size_absolute = static_cast<size_t>(
		    std::round(max * this->partition_size - min * this->partition_size));
		size_t partition_no =
		    (this->counter / this->change_freq) % this->partition_count;
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
