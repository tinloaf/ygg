//
// Created by Lukas Barth on 09.08.17.
//

#ifndef YGG_UTIL_HPP

#include <iterator>
#include <type_traits>

namespace ygg {

namespace utilities {

template <class T>
class TypeHolder {
public:
	using type = T;
};

// From
// https://stackoverflow.com/questions/31762958/check-if-class-is-a-template-specialization#31763111
template <class T, template <class...> class Template>
struct is_specialization : std::false_type
{
};

template <template <class...> class Template, class... Args>
struct is_specialization<Template<Args...>, Template> : std::true_type
{
};

template <class T, template <std::size_t> class Template>
struct is_numeric_specialization : std::false_type
{
};

template <template <std::size_t> class Template, std::size_t N>
struct is_numeric_specialization<Template<N>, Template> : std::true_type
{
};

template <class... Ts>
void
throw_away(Ts...)
{}

class NotFoundMarker {
public:
	static constexpr std::size_t value = 0;
};

template <template <std::size_t> class TMPL>
constexpr auto
get_value_if_present_func()
{
	return TypeHolder<NotFoundMarker>{};
}

template <template <std::size_t> class TMPL, class T, class... Rest>
constexpr auto get_value_if_present_func(
    typename std::enable_if<is_numeric_specialization<T, TMPL>{}, bool>::type
        dummy = true)
{
	(void)dummy;
	return TypeHolder<T>{};
}

template <template <std::size_t> class TMPL, class T, class... Rest>
constexpr auto get_value_if_present_func(
    typename std::enable_if<!is_numeric_specialization<T, TMPL>{}, bool>::type
        dummy = true)
{
	(void)dummy;
	return get_value_if_present_func<TMPL, Rest...>();
}

template <template <std::size_t> class TMPL, class... Ts>
class get_value_if_present {
private:
	using type =
	    typename decltype(get_value_if_present_func<TMPL, Ts...>())::type;

public:
	constexpr static bool found = !std::is_same<type, NotFoundMarker>::value;
	constexpr static std::size_t value = type::value;
};

template <template <std::size_t> class TMPL, std::size_t DEFAULT, class... Ts>
class get_value_if_present_else_default {
private:
	using type =
	    typename decltype(get_value_if_present_func<TMPL, Ts...>())::type;

public:
	constexpr static bool found = !std::is_same<type, NotFoundMarker>::value;
	constexpr static std::size_t value = found ? type::value : DEFAULT;
};

template <template <class> class TMPL, class Default>
constexpr auto
get_type_if_present_func()
{
	return TypeHolder<Default>{};
}

template <template <class> class TMPL, class Default, class T, class... Rest>
constexpr auto get_type_if_present_func(
    typename std::enable_if<is_specialization<T, TMPL>{}, bool>::type dummy =
        true)
{
	(void)dummy;
	return TypeHolder<T>{};
}

template <template <class> class TMPL, class Default, class T, class... Rest>
constexpr auto get_type_if_present_func(
    typename std::enable_if<!is_specialization<T, TMPL>{}, bool>::type dummy =
        true)
{
	(void)dummy;
	return get_type_if_present_func<TMPL, Default, Rest...>();
}

template <template <class> class TMPL, class Default, class... Ts>
class get_type_if_present {
public:
	using type =
	    typename decltype(get_type_if_present_func<TMPL, Default, Ts...>())::type;
};

/* Return true if any of the arguments returns true */
template <class... Bools>
constexpr bool any_of(Bools... arguments);

template <class First, class... Remainder>
constexpr bool
any_of(First first, Remainder... remainder)
{
	return first || any_of(remainder...);
}

template <>
constexpr bool
any_of()
{
	return false;
}

/* @brief A class providing an iterator over the integers 1, 2, … <n>
 */
// TODO this does not uphold the multipass guarantee (point 1). Check how boost
// does this.
template <class T>
class IntegerRange {
public:
	IntegerRange(T start_in, T stop_in) : start(start_in), stop(stop_in) {}

	class iterator {
	public:
		typedef T difference_type;
		typedef T value_type;
		typedef const T & reference;
		typedef T * pointer;
		typedef std::random_access_iterator_tag iterator_category;

		iterator(T start) : val(start) {}
		iterator(const iterator & other) : val(other.val) {}
		iterator() : val() {}

		iterator &
		operator=(const iterator & other)
		{
			val = other.val;
		};

		bool
		operator==(const iterator & other) const
		{
			return this->val == other.val;
		}
		bool
		operator!=(const iterator & other) const
		{
			return this->val != other.val;
		}

		iterator
		operator++(int)
		{
			iterator cpy = *this;
			this->val++;
			return cpy;
		}
		iterator &
		operator++()
		{
			this->val++;
			return *this;
		}
		iterator &
		operator+=(size_t steps)
		{
			this->val += steps;
			return *this;
		}
		iterator
		operator+(size_t steps) const
		{
			iterator cpy = *this;
			cpy += steps;
			return cpy;
		}

		iterator
		operator--(int)
		{
			iterator cpy = *this;
			this->val--;
			return cpy;
		}
		iterator &
		operator--()
		{
			this->val--;
			return *this;
		}
		iterator &
		operator-=(size_t steps)
		{
			this->val -= steps;
			return *this;
		}
		iterator
		operator-(size_t steps) const
		{
			iterator cpy = *this;
			cpy -= steps;
			return cpy;
		}

		difference_type
		operator-(const iterator & other) const
		{
			return this->val - other.val;
		}
		bool
		operator<(const iterator & other) const
		{
			return this->val < other.val;
		}
		bool
		operator<=(const iterator & other) const
		{
			return this->val <= other.val;
		}
		bool
		operator>(const iterator & other) const
		{
			return this->val > other.val;
		}
		bool
		operator>=(const iterator & other) const
		{
			return this->val >= other.val;
		}

		reference operator[](size_t n) { return this->val + n; }

		reference operator*() const { return this->val; }

		pointer operator->() const { return &(this->val); }

	private:
		T val;
	};


	iterator begin() const {
		return iterator(start);
	}

	iterator end() const {
		return iterator(stop);
	}

private:
	const T start;
	const T stop;
};

} // namespace utilities

// TODO why is it this namespace?
namespace rbtree_internal {

/**
 * @brief A more flexible version of std::less
 *
 * This is a more flexible version of std::less, which allows to compare two
 * objects of different types T1 and T2, as long as operator<(T1, T2) and
 * operator<(T2, T1) is defined.
 */
class flexible_less {
public:
	template <class T1, class T2>
	constexpr bool
	operator()(const T1 & lhs, const T2 & rhs) const
	{
		return lhs < rhs;
	}
};

/*
 * This is inspired by
 *
 * https://stackoverflow.com/questions/34099597/check-if-a-type-is-passed-in-variadic-template-parameter-pack
 *
 * and should be converted to std::disjunction as soon as C++17 becomes
 * available on reasonable compilers.
 */
// template<typename QueryT, typename ...Ts>
// constexpr bool pack_contains();

template <typename QueryT>
constexpr bool
pack_contains()
{
	return false;
}

// Forward
template <typename QueryT, typename First, typename... Rest>
constexpr bool pack_contains();

template <typename QueryT, bool found, typename... Rest>
constexpr typename std::enable_if<found, bool>::type
pack_contains_forward()
{
	return true;
}

template <typename QueryT, bool found, typename... Rest>
constexpr typename std::enable_if<!found, bool>::type
pack_contains_forward()
{
	return pack_contains<QueryT, Rest...>();
}

template <typename QueryT, typename First, typename... Rest>
constexpr bool
pack_contains()
{
	return pack_contains_forward<QueryT, std::is_same<QueryT, First>::value,
	                             Rest...>();
}

/*
 * Generic class to contain a template parameter pack
 */
template <class...>
struct pack
{
};

/*
 * Template parameter currying
 */
// template<template <class ...> class In, class ... CurriedTs>
// struct pack_curry {
//	template<class ... Ts>
//	using type = In<CurriedTs ..., Ts ...>;
//};

/*
 * Removes RemoveMe... from the beginning of the parameter pack in From<InTs...>
 */
// template<class From, class ... RemoveMe>
// struct pack_remove_prefix;
//
// template<template <class ...> class From,
//				 class ... Ts,
//				 class ... RemoveMe>
// struct pack_remove_prefix<From<RemoveMe..., Ts...>, RemoveMe...>
//{
//	using type = pack<Ts...>;
//};

/*
 * Removes RemoveMe... from the end of the parameter pack in From<InTs...>
 */
// template<class From, class ... RemoveMe>
// struct pack_remove_postfix;
//
// template<template <class ...> class From,
//                              class ... Ts,
//                              class ... RemoveMe>
// struct pack_remove_postfix<From<Ts..., RemoveMe...>, RemoveMe...>
//{
//	using type = pack<Ts...>;
//};

/*
 * Passes around a parameter pack.
 */
template <class From, template <class...> class To>
struct pass_pack;

template <template <class...> class From, template <class...> class To,
          class... Ts>
struct pass_pack<From<Ts...>, To>
{
	using type = To<Ts...>;
};

/*
 * Cut out a number of parameters and pass from one template to another
 */
// template<class From,
//				 template <class ...> class To,
//				 class prefix_skip_pack,
//				 class postfix_skip_pack>
// struct pack_transfer {
//	using type = pass_pack<pack_remove_postfix<
//					pack_remove_prefix<From,
// prefix_skip_pack>::type,
// postfix_skip_pack>::type,
// To>::type;
//};

} // namespace rbtree_internal
} // namespace ygg

#define YGG_UTIL_HPP

#endif // YGG_UTIL_HPP
