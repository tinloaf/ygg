//
// Created by Lukas Barth on 09.08.17.
//

#ifndef YGG_OPTIONS_HPP
#define YGG_OPTIONS_HPP

#include <type_traits>

#include "util.hpp"

namespace ygg {

/**
 * @brief Contains possible options for the RBTree
 *
 * This class contains all possible options (that can each either be set or not set) for an
 * RBTree. See the TreeOptions documentation on how to set them. See the individual inner classes
 * for information about what each option does.
 */
class TreeFlags {
public:
	/**
	 * @brief RBTree option: Allow multiple elements that compare equally
	 *
	 * If this option is set, an RBTree allows you to insert elements a and b for which
	 * "Compare(a,b) == Compare(b,a) == false" holds, i.e., which compare equally to each other.
	 * This option is set by default. You can unset it if you know that this case will never
	 * happen, saving a little space and gaining a little performance.
	 */
	class MULTIPLE {};
	/**
	 * @brief RBTree option: Support order queries
	 *
	 * If this flag is set, the RBTree efficiently supports order queries, i.e., queries of the
	 * form "is a before b in the tree". Please note that this only makes sense if MULTIPLE is also
	 * set --- otherwise, Compare(a,b) answers that question for every pair (a,b). For elements that
	 * compare equally (i.e. Compare(a,b) == Compare(b,a) == false), the hinted version of
	 * RBTree::insert allows you to enforce a certain order on equal elements. TODO link to method
	 */
	class ORDER_QUERIES {};
	/**
	 * @brief RBTree / List option: support size() in O(1)
	 *
	 * If this flag is set, the data structures keep track of the number of elements in them,
	 * thereby supporting size() queries in O(1). This requires a little space and will slow
	 * down insert and remove operations minimally.
	 */
	class CONSTANT_TIME_SIZE {};
};

/**
 * @brief Class holding the options for an RBTree
 *
 * This class acts as a container for the options of an RBTree. Note that this class should never
 * (and in fact, cannot be) instantiated. All options are passed as template parameters, and the
 * class itself is in turn passed as template parameter to RBTree and RBTreeNodeBase.
 *
 * Example, setting both the MULTIPLE and ORDER_QUERIES options:
 *
 * @code{.c++}
 * using MyTreeOptions = TreeOptions<TreeFlags::MULTIPLE, TreeFlags::ORDER_QUERIES>;
 * class Node : public RBTreeNodeBase<Node, MyTreeOptions> { … };
 * using MyTree = RBTree<Node, RBDefaultNodeTraits<Node>, MyTreeOptions>;
 * @endcode
 *
 * @tparam Opts List of the options that should be set in the tree.
 */
template<class ... Opts>
class TreeOptions
{
public:
	/// @cond INTERNAL
	static constexpr bool multiple = utilities::pack_contains<TreeFlags::MULTIPLE, Opts...>();
	static constexpr bool order_queries = utilities::pack_contains<TreeFlags::ORDER_QUERIES,
	                                                               Opts...>();
	static constexpr bool constant_time_size = utilities::pack_contains<TreeFlags::CONSTANT_TIME_SIZE,
	                                                                    Opts...>();
	/// @endcond
private:
	TreeOptions(); // Instantiation not allowed
};

using DefaultOptions = TreeOptions<TreeFlags::MULTIPLE, TreeFlags::CONSTANT_TIME_SIZE>;

} // namespace ygg

#endif //YGG_OPTIONS_HPP
