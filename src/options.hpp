//
// Created by Lukas Barth on 09.08.17.
//

#ifndef YGG_OPTIONS_HPP
#define YGG_OPTIONS_HPP

#include "util.hpp"

namespace ygg {

class TreeFlags {
public:
	class MULTIPLE {};
	class ORDER_QUERIES {};
};

template<class ... Opts>
class TreeOptions
{
public:
	static constexpr bool multiple = utilities::pack_contains<TreeFlags::MULTIPLE, Opts...>();
	static constexpr bool order_queries = utilities::pack_contains<TreeFlags::ORDER_QUERIES,
	                                                               Opts...>();
};

}; // namespace ygg

#endif //YGG_OPTIONS_HPP
