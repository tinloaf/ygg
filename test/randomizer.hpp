#pragma once
#ifndef YGG_RANDOMIZER_HPP
#define YGG_RANDOMIZER_HPP

#include <cstddef>
#include <limits>
#include <random>

namespace ygg {
namespace testing {
namespace utilities {

class Randomizer {
public:
  explicit Randomizer(size_t seed)
    : rng(seed), distr(0, std::numeric_limits<size_t>::max())
  {}
  
  Randomizer()
    : rng(), distr(0, std::numeric_limits<size_t>::max())
  {
    std::random_device rd;
    rng = std::mt19937(rd());
  }
  
  using result_type = size_t;

  size_t
  min() const noexcept
  {
    return std::numeric_limits<size_t>::min();
  }

  size_t
  max() const noexcept
  {
    return std::numeric_limits<size_t>::max();
  }

  size_t
  operator()()
  {
    return this->distr(this->rng);
  }

private:
  std::mt19937 rng;
  std::uniform_int_distribution<size_t> distr;
};

} // namespace util
} // namespace testing
} // namespace ygg

#endif
