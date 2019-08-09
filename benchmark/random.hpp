#ifndef YGG_RANDOM_HPP
#define YGG_RANDOM_HPP

#include "../src/util.hpp"

#include <random>
#include <unordered_map>
class Randomizer {
public:
	virtual int generate(int min, int max) = 0;
	Randomizer() = delete;
	virtual ~Randomizer() = default;
	unsigned long get_seed() const noexcept;

	virtual int get_default_max() const noexcept = 0;
	virtual const char * get_name() const noexcept = 0;

protected:
	Randomizer(unsigned long seed);
	std::mt19937 rng;
	const unsigned long stored_seed;
};

/*
 * This is a C++ port of the rejection-based sampler of
 * https://github.com/apache/commons-math/blob/138f84bfa5d36c8f6e2825640af1ed82daa9dc1d/src/main/java/org/apache/commons/math4/distribution/ZipfDistribution.java
 *
 * Which in term is an implementation of
 *   Wolfgang Hörmann and Gerhard Derflinger
 *   "Rejection-inversion to generate variates from monotone discrete
 * distributions." ACM Transactions on Modeling and Computer Simulation
 * (TOMACS) 6.3 (1996): 169-184.
 */

class ZipfDistr : public Randomizer {
public:
	ZipfDistr(unsigned long seed, double exponent);
	virtual ~ZipfDistr() = default;

	virtual int generate(int min, int max) override;
	virtual int get_default_max() const noexcept override;
	virtual const char * get_name() const noexcept override;

private:
	class ZipfSampler {
	public:
		ZipfSampler(double exponent, size_t number_of_elements, std::mt19937 & rng);

		int generate();

	private:
		const double exponent;
		const size_t number_of_elements;
		std::mt19937 & rng;

		/* Computed constants */
		double h_integral_x1; // h_integral(1.5) - 1
		double
		    h_integral_number_of_elements; // h_integral(number_of_elements + 0.5)
		double s; // 2 - h_integral_inverse(h_integal(2.5) - h(2))

		double h(double x) const noexcept;
		double h_integral(double x) const noexcept;
		double h_integral_inverse(double x) const noexcept;

		// log(1+x)/x
		double helper1(double x) const noexcept;
		// (exp(x)-1)/x
		double helper2(double x) const noexcept;
	};

	ZipfSampler & get_sampler(int min, int max);
	double exponent;
};

class MaekinenSkewedDistr : public Randomizer {
public:
	MaekinenSkewedDistr(unsigned long seed, size_t n, size_t change_freq,
	                    size_t partition_count = 2, double partition_size = 0.1);
	virtual ~MaekinenSkewedDistr() = default;

	virtual int generate(int min, int max) override;
	virtual int get_default_max() const noexcept override;
	virtual const char * get_name() const noexcept override;

private:
	size_t n;
	size_t change_freq;
	size_t partition_count;
	double partition_size;
	size_t counter;
};

class UniformDistr : public Randomizer {
public:
	UniformDistr(unsigned long seed);
	virtual ~UniformDistr() = default;

	virtual int generate(int min, int max) override;
	virtual int get_default_max() const noexcept override;
	virtual const char * get_name() const noexcept override;
};

#endif
