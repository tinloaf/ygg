//
// Created by Lukas Barth on 09.08.17.
//

#ifndef YGG_UTIL_HPP

namespace ygg {
namespace utilities {

/*
 * This is inspired by
 *
 * https://stackoverflow.com/questions/34099597/check-if-a-type-is-passed-in-variadic-template-parameter-pack
 *
 * and should be converted to std::disjunction as soon as C++17 becomes available on reasonable
 * compilers.
 */
//template<typename QueryT, typename ...Ts>
//constexpr bool pack_contains();

template<typename QueryT>
constexpr bool pack_contains() {
	return false;
}

template<typename QueryT, bool found, typename ...Rest>
constexpr typename std::enable_if<found, bool>::type pack_contains_forward() {
	return true;
}

template<typename QueryT, bool found, typename ...Rest>
constexpr typename std::enable_if<! found, bool>::type pack_contains_forward() {
	return pack_contains<QueryT, Rest...>();
}



template<typename QueryT, typename First, typename ...Rest>
constexpr bool pack_contains() {
	return pack_contains_forward<QueryT, std::is_same<QueryT, First>::value, Rest...>();
}

} // namespace utilities
} // namespace ygg

#define YGG_UTIL_HPP

#endif //YGG_UTIL_HPP
