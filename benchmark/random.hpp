#ifndef YGG_RANDOM_HPP
#define YGG_RANDOM_HPP

#include "../src/util.hpp"

#include <random>
#include <unordered_map>
class Randomizer {
public:
	virtual int generate(int min, int max) = 0;
	Randomizer() = delete;

protected:
	Randomizer(unsigned long seed);
	std::mt19937 rng;
};

class ZipfDistr : public Randomizer {
public:
	ZipfDistr(unsigned long seed, double s);

	virtual int generate(int min, int max) override;

private:
	double s;
	std::uniform_real_distribution<double> cdf_dist;
	// TODO cache values
	double ghn(size_t n, double m);
	size_t inverse_ghn_on_n(double ghn, double m);

	using GhnCache = std::unordered_map<std::pair<size_t, double>, double,
	                                    ygg::utilities::pair_hash>;

	using InverseGhnCache = std::unordered_map<std::pair<double, double>, size_t,
	                                           ygg::utilities::pair_hash>;
};

class MaekinenSkewedDistr : public Randomizer {
public:
	MaekinenSkewedDistr(unsigned long seed, size_t n, size_t change_freq,
	                    size_t partition_count = 2, double partition_size = 0.1);

	virtual int generate(int min, int max) override;

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
	virtual int generate(int min, int max) override;
};

#endif
