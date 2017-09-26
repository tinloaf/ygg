//
// Created by Lukas Barth on 09.08.17.
//

#ifndef YGG_UTIL_HPP

#include <type_traits>

namespace ygg {
namespace utilities {

/**
 * @brief A more flexible version of std::less
 *
 * This is a more flexible version of std::less, which allows to compare two objects of different
 * types T1 and T2, as long as operator<(T1, T2) and operator<(T2, T1) is defined.
 */
class flexible_less {
public:
	template<class T1, class T2>
	constexpr bool operator()(const T1 &lhs, const T2 &rhs) const
	{
		return lhs < rhs;
	}
};

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
