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

	std::unordered_map<std::pair<size_t, double>, double,
	                   ygg::utilities::pair_hash>
	    ghn_cache;
	std::unordered_map<std::pair<double, double>, size_t,
	                   ygg::utilities::pair_hash>
	    inverse_ghn_cache;
};

class UniformDistr : public Randomizer {
public:
	UniformDistr(unsigned long seed);
	virtual int generate(int min, int max) override;
};

#endif
