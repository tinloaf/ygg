//
// Created by Lukas Barth on 09.08.17.
//

#ifndef YGG_UTIL_HPP

#include <type_traits>

namespace ygg {
	
namespace utilities {

template<class ... Ts>
void throw_away(Ts ...) {}

} // namespace ygg::utilities
	
namespace rbtree_internal {

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

// Forward
template<typename QueryT, typename First, typename ...Rest>
constexpr bool pack_contains();

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

/*
 * Generic class to contain a template parameter pack
 */
template<class ...> struct pack {};

/*
 * Template parameter currying
 */
//template<template <class ...> class In, class ... CurriedTs>
//struct pack_curry {
//	template<class ... Ts>
//	using type = In<CurriedTs ..., Ts ...>;
//};

/*
 * Removes RemoveMe... from the beginning of the parameter pack in From<InTs...>
 */
//template<class From, class ... RemoveMe>
//struct pack_remove_prefix;
//
//template<template <class ...> class From,
//				 class ... Ts,
//				 class ... RemoveMe>
//struct pack_remove_prefix<From<RemoveMe..., Ts...>, RemoveMe...>
//{
//	using type = pack<Ts...>;
//};

/*
 * Removes RemoveMe... from the end of the parameter pack in From<InTs...>
 */
//template<class From, class ... RemoveMe>
//struct pack_remove_postfix;
//
//template<template <class ...> class From,
//                              class ... Ts,
//                              class ... RemoveMe>
//struct pack_remove_postfix<From<Ts..., RemoveMe...>, RemoveMe...>
//{
//	using type = pack<Ts...>;
//};

/*
 * Passes around a parameter pack.
 */
template<class From, template <class ...> class To>
struct pass_pack;

template<template <class ...> class From,
				template <class ...> class To,
				                      class ... Ts>
struct pass_pack<From<Ts...>, To>
{
	using type = To<Ts...>;
};

/*
 * Cut out a number of parameters and pass from one template to another
 */
//template<class From,
//				 template <class ...> class To,
//				 class prefix_skip_pack,
//				 class postfix_skip_pack>
//struct pack_transfer {
//	using type = pass_pack<pack_remove_postfix<
//					pack_remove_prefix<From, prefix_skip_pack>::type,
//					                   postfix_skip_pack>::type,
//								          To>::type;
//};

} // namespace utilities
} // namespace ygg

#define YGG_UTIL_HPP

#endif //YGG_UTIL_HPP
