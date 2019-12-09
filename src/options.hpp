//
// Created by Lukas Barth on 09.08.17.
//

#ifndef YGG_OPTIONS_HPP
#define YGG_OPTIONS_HPP

#include "util.hpp"

#include <cstddef>
#include <limits>
#include <type_traits>

namespace ygg {
/**
 * @brief Contains possible options for the RBTree
 *
 * This class contains all possible options (that can each either be set or not
 * set) for an RBTree. See the TreeOptions documentation on how to set them. See
 * the individual inner classes for information about what each option does.
 */
class TreeFlags {
public:
	/**
	 * @brief RBTree option: Allow multiple elements that compare equally
	 *
	 * If this option is set, an RBTree allows you to insert elements a and b for
	 * which "Compare(a,b) == Compare(b,a) == false" holds, i.e., which compare
	 * equally to each other. This option is set by default. You can unset it if
	 * you know that this case will never happen, saving a little space and
	 * gaining a little performance.
	 */
	class MULTIPLE {
	};
	/**
	 * @brief RBTree option: Support order queries
	 *
	 * If this flag is set, the RBTree efficiently supports order queries, i.e.,
	 * queries of the form "is a before b in the tree". Please note that this only
	 * makes sense if MULTIPLE is also set --- otherwise, Compare(a,b) answers
	 * that question for every pair (a,b). For elements that compare equally (i.e.
	 * Compare(a,b) == Compare(b,a) == false), the hinted version of
	 * RBTree::insert allows you to enforce a certain order on equal elements.
	 * TODO link to method
	 */
	class ORDER_QUERIES {
	};

	/**
	 * @brief RBTree / List option: support size() in O(1)
	 *
	 * If this flag is set, the data structures keep track of the number of
	 * elements in them, thereby supporting size() queries in O(1). This requires
	 * a little space and will slow down insert and remove operations minimally.
	 */
	class CONSTANT_TIME_SIZE {
	};

	/**
	 * @brief Make the erase() method adhere to STL conventions
	 *
	 * If this flag is set, the erase() method of the various trees has a behavior
	 * that is in line with STL's std::set (or std::multiset), returning an
	 * iterator to the next element in the tree, and removing all nodes that match
	 * a key. See documentation on the erase() method for details.
	 */
	class STL_ERASE {
	};

	/**
	 * @brief RBTree option: Indicates that color information should be compressed
	 * into the parent pointer
	 *
	 * This saves a little memory and can in fact increase speed if your nodes are
	 * very small. It uses some pointer magic which is technically not standard
	 * compliant but should work on almost all systems.
	 */
	class COMPRESS_COLOR {
	};

	/**
	 * @brief Zip Tree Option: Indicates that nodes' ranks should be derived from
	 * a std::hash hash of the node.
	 */
	class ZTREE_USE_HASH {
	};

	/**
	 * @brief Zip Tree Option: Causes ranks to be stored in nodes and sets their
	 * type.
	 *
	 * The ranks of the nodes need not necessarily be stored at the nodes. If you
	 * e.g. generate ranks from hashes, they can be recomputed whenever needed.
	 * This saves some space in the nodes.
	 *
	 * @warning If you do not enable ZTREE_USE_HASH, you *must* set
	 * ZTREE_RANK_TYPE.
	 *
	 * @tparam T The type that you want to use to store the rank. Usually, uint8_t
	 * is sufficient.
	 */
	template <class T>
	class ZTREE_RANK_TYPE {
	public:
		using type = T;
	};

	/**
	 * @brief Zip Tree Option: Apply universal hashing to compute node ranks. This
	 * sets the coefficient.
	 *
	 * The default way of computing ranks (from hashes) can apply universal
	 * hashing to (to some extents) guarantee a geometric distribution of the
	 * nodes' ranks. The universal hashing of a std::hash-value x is done via:
	 *
	 *   h = (x * <coefficient>) % <modul>
	 *
	 * This parameter sets the coefficient.
	 *
	 * If you do not set the modul via ZTREE_RANK_HASH_UNIVERSALIZE_MODUL,
	 * this results in no modulo arithmetic being done (mathematically equivalent
	 * to setting ZTREE_RANK_HASH_UNIVERSALIZE_MODUL to
	 * std::numeric_limits<size_t>::max()). If you set
	 * ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT to a random odd number, this
	 * results in 2/m-almost-universality of the random values, but yields less
	 * costly math operations.
	 *
	 * @tparam coefficient_in The desired coefficient.
	 */
	template <size_t coefficient_in>
	class ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT {
	public:
		constexpr static size_t value = coefficient_in;
	};

	/**
	 * @brief Zip Tree Option: Apply universal hashing to compute node ranks. This
	 * sets the modul.
	 *
	 * The default way of computing ranks (from hashes) can apply universal
	 * hashing to (to some extents) guarantee a geometric distribution of the
	 * nodes' ranks. The universal hashing of a std::hash-value x is done via:
	 *
	 *   h = (x * <coefficient>) % <modul>
	 *
	 * This parameter sets the modul. Note that setting the modul to
	 * std::numeric_limits<size_t>::max() is not useful.
	 *
	 * If you do not set the modul, but you set
	 * ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT, this results in no modulo
	 * arithmetic being done (mathematically equivalent to setting
	 * ZTREE_RANK_HASH_UNIVERSALIZE_MODUL to std::numeric_limits<size_t>::max()).
	 * Doing this results in 2/m-almost-universality of the random values, but
	 * yields less costly math operations.
	 *
	 * @warning Universal hashing is only activated if you also set
	 * ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT.
	 *
	 * @tparam coefficient_in The desired modul. You might want to chose a prime.
	 */
	template <size_t modul_in>
	class ZTREE_RANK_HASH_UNIVERSALIZE_MODUL {
	public:
		constexpr static size_t value = modul_in;
	};

	/**
	 * @brief Weight Balanced Tree Option: Sets the numerator of the Delta balance
	 * parameter
	 *
	 * This sets the numerator of the Delta balance parameter. See
	 * https://www.cambridge.org/core/journals/journal-of-functional-programming/article/balancing-weightbalanced-trees/7281C4DE7E56B74F2D13F06E31DCBC5B
	 * for information on Delta.
	 *
	 * @tparam delta_numerator The numerator of Delta
	 */
	template <size_t delta_numerator>
	class WBT_DELTA_NUMERATOR {
	public:
		constexpr static size_t value = delta_numerator;
	};

	/**
	 * @brief Weight Balanced Tree Option: Sets the numerator of the Gamma balance
	 * parameter
	 *
	 * This sets the numerator of the Gamma balance parameter. See
	 * https://www.cambridge.org/core/journals/journal-of-functional-programming/article/balancing-weightbalanced-trees/7281C4DE7E56B74F2D13F06E31DCBC5B
	 * for information on Gamma.
	 *
	 * @tparam delta_numerator The numerator of Gamma
	 */
	template <size_t gamma_numerator>
	class WBT_GAMMA_NUMERATOR {
	public:
		constexpr static size_t value = gamma_numerator;
	};

	/**
	 * @brief Weight Balanced Tree Option: Sets the denominator of the Delta
	 * balance parameter
	 *
	 * This sets the denominator of the Delta balance parameter. See
	 * https://www.cambridge.org/core/journals/journal-of-functional-programming/article/balancing-weightbalanced-trees/7281C4DE7E56B74F2D13F06E31DCBC5B
	 * for information on Delta.
	 *
	 * Note that setting this to 1 (i.e., chosing an integral Delta) improves
	 * performance.
	 *
	 * @tparam delta_numerator The denominator of Delta
	 */
	template <size_t delta_denominator>
	class WBT_DELTA_DENOMINATOR {
	public:
		constexpr static size_t value = delta_denominator;
	};

	/**
	 * @brief Weight Balanced Tree Option: Sets the denominator of the Gamma
	 * balance parameter
	 *
	 * This sets the denominator of the Gamma balance parameter. See
	 * https://www.cambridge.org/core/journals/journal-of-functional-programming/article/balancing-weightbalanced-trees/7281C4DE7E56B74F2D13F06E31DCBC5B
	 * for information on Gamma.
	 *
	 * Note that setting this to 1 (i.e., chosing an integral Gamma) improves
	 * performance.
	 *
	 * @tparam delta_numerator The denominator of Gamma
	 */
	template <size_t gamma_denominator>
	class WBT_GAMMA_DENOMINATOR {
	public:
		constexpr static size_t value = gamma_denominator;
	};

	class WBT_SINGLE_PASS {
	};

	/******************************************************
	 * Micro-Optimization Options
	 ******************************************************/
	/**
	 * @brief Enables tricks to avoid conditional branching
	 *
	 * Enabling this will cause the various data structures to enable tricks to
	 * avoid conditional branching, using e.g. pointer arithmetic instead. This
	 * can siginificantly increase performance on certain architectures, e.g.
	 * modern SkyLake CPUs. However, if you enable this, make sure you profile
	 * your performance to determine whether this actually helps with your
	 * specific workload.
	 */
	class MICRO_AVOID_CONDITIONALS {
	};

	/******************************************************
	 * Benchmarking Helpers
	 ******************************************************/
	template <class SequenceInterface>
	class BENCHMARK_SEQUENCE_INTERFACE {
	public:
		using type = SequenceInterface;
	};
};

/**
 * @brief Class holding the options for the data structures in this library.
 *
 * This class acts as a container for the options of all the data structures in
 * this library. Note that this class should never (and in fact, cannot be)
 * instantiated. All options are passed as template parameters, and the class
 * itself is in turn passed as template parameter to the data structures.
 *
 * Example, setting both the MULTIPLE and ORDER_QUERIES options for a red-black
 * tree:
 *
 * @code{.c++}
 * using MyTreeOptions = TreeOptions<TreeFlags::MULTIPLE,
 * TreeFlags::ORDER_QUERIES>; class Node : public RBTreeNodeBase<Node,
 * MyTreeOptions> { … }; using MyTree = RBTree<Node, RBDefaultNodeTraits,
 * MyTreeOptions>;
 * @endcode
 *
 * @tparam Opts List of the options that should be set in the tree.
 */
template <class... Opts>
class TreeOptions {
private:
	static constexpr size_t ztree_universalize_modul_default =
	    std::numeric_limits<size_t>::max();
	static constexpr size_t ztree_universalize_coefficient_default = 1103515245;

	static constexpr double wbt_delta_default = 2.41421356237; // 1 + sqrt(2)
	static constexpr double wbt_gamma_default = 1.41421356237; // sqrt(2)

	struct IntegralTypeHolder
	{
		using type = size_t;
	};
	struct FloatTypeHolder
	{
		using type = double;
	};

	constexpr static auto
	compute_wbt_gamma_type()
	{
		if constexpr (OptPack::template has_tmpl_size_t<
		                  TreeFlags::WBT_DELTA_DENOMINATOR>() &&
		              OptPack::template has_tmpl_size_t<
		                  TreeFlags::WBT_DELTA_NUMERATOR>() &&
		              utilities::get_value_if_present<
		                  TreeFlags::WBT_GAMMA_DENOMINATOR, Opts...>::value == 1) {
			return IntegralTypeHolder();
		} else {
			return FloatTypeHolder();
		}
	}

	constexpr static auto
	compute_wbt_delta_type()
	{
		if constexpr (OptPack::template has_tmpl_size_t<
		                  TreeFlags::WBT_DELTA_DENOMINATOR>() &&
		              OptPack::template has_tmpl_size_t<
		                  TreeFlags::WBT_DELTA_NUMERATOR>() &&
		              utilities::get_value_if_present<
		                  TreeFlags::WBT_DELTA_DENOMINATOR, Opts...>::value == 1) {
			return IntegralTypeHolder();
		} else {
			return FloatTypeHolder();
		}
	}

	constexpr static auto
	compute_sequence_interface_type()
	{
		using T = typename utilities::get_type_if_present<
		    TreeFlags::BENCHMARK_SEQUENCE_INTERFACE, void, Opts...>::type;

		if constexpr (!std::is_same_v<T, void>) {
			return utilities::TypeHolder<typename T::type>{};
		} else {
			return utilities::TypeHolder<void>{};
		}
	}

	using OptPack = utilities::Pack<Opts...>;

public:
	/// @cond INTERNAL

	static constexpr bool multiple = OptPack::template has<TreeFlags::MULTIPLE>();
	static constexpr bool order_queries =
	    OptPack::template has<TreeFlags::ORDER_QUERIES>();
	static constexpr bool constant_time_size =
	    OptPack::template has<TreeFlags::CONSTANT_TIME_SIZE>();
	static constexpr bool compress_color =
	    OptPack::template has<TreeFlags::COMPRESS_COLOR>();
	static constexpr bool ztree_use_hash =
	    OptPack::template has<TreeFlags::ZTREE_USE_HASH>();
	static constexpr bool stl_erase =
	    OptPack::template has<TreeFlags::STL_ERASE>();
	using ztree_rank_type =
	    typename utilities::get_type_if_present<TreeFlags::ZTREE_RANK_TYPE, void,
	                                            Opts...>::type;

	static constexpr bool ztree_universalize_lincong =
	    (utilities::get_value_if_present<
	         TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_MODUL, Opts...>::found &&
	     utilities::get_value_if_present<
	         TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT,
	         Opts...>::found);

	static constexpr bool ztree_universalize_multiply =
	    ((!utilities::get_value_if_present<
	         TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_MODUL, Opts...>::found) &&
	     utilities::get_value_if_present<
	         TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT,
	         Opts...>::found);

	static constexpr bool ztree_store_rank =
	    !std::is_same<ztree_rank_type, void>::value;

	static constexpr size_t ztree_universalize_modul =
	    utilities::get_value_if_present_else_default<
	        TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_MODUL,
	        ztree_universalize_modul_default, Opts...>::value;

	static constexpr size_t ztree_universalize_coefficient =
	    utilities::get_value_if_present_else_default<
	        TreeFlags::ZTREE_RANK_HASH_UNIVERSALIZE_COEFFICIENT,
	        ztree_universalize_coefficient_default, Opts...>::value;

	using WBTDeltaT =
	    typename decltype(TreeOptions<Opts...>::compute_wbt_delta_type())::type;
	static constexpr WBTDeltaT
	wbt_delta()
	{
		if constexpr (OptPack::template has_tmpl_size_t<
		                  TreeFlags::WBT_DELTA_DENOMINATOR>() &&
		              OptPack::template has_tmpl_size_t<
		                  TreeFlags::WBT_DELTA_NUMERATOR>() &&
		              (utilities::get_value_if_present<
		                   TreeFlags::WBT_DELTA_DENOMINATOR, Opts...>::value !=
		               0)) {
			return (utilities::get_value_if_present<TreeFlags::WBT_DELTA_NUMERATOR,
			                                        Opts...>::value /
			        utilities::get_value_if_present<TreeFlags::WBT_DELTA_DENOMINATOR,
			                                        Opts...>::value);
		} else {
			return wbt_delta_default;
		}
	}

	static std::string
	wbt_delta_str()
	{
		if constexpr (OptPack::template has_tmpl_size_t<
		                  TreeFlags::WBT_DELTA_DENOMINATOR>() &&
		              OptPack::template has_tmpl_size_t<
		                  TreeFlags::WBT_DELTA_NUMERATOR>() &&
		              (utilities::get_value_if_present<
		                   TreeFlags::WBT_DELTA_DENOMINATOR, Opts...>::value !=
		               0)) {
			if (utilities::get_value_if_present<TreeFlags::WBT_DELTA_DENOMINATOR,
			                                    Opts...>::value != 1) {
				return std::to_string(utilities::get_value_if_present<
				                      TreeFlags::WBT_DELTA_NUMERATOR, Opts...>::value) +
				       std::string("/") +
				       std::to_string(
				           utilities::get_value_if_present<
				               TreeFlags::WBT_DELTA_DENOMINATOR, Opts...>::value);
			} else {
				return std::to_string(
				    utilities::get_value_if_present<TreeFlags::WBT_DELTA_NUMERATOR,
				                                    Opts...>::value);
			}
		} else {
			return std::string("1+sqrt(2)");
		}
	}

	using WBTGammaT =
	    typename decltype(TreeOptions<Opts...>::compute_wbt_gamma_type())::type;
	static constexpr WBTGammaT
	wbt_gamma()
	{
		if constexpr (OptPack::template has_tmpl_size_t<
		                  TreeFlags::WBT_DELTA_DENOMINATOR>() &&
		              OptPack::template has_tmpl_size_t<
		                  TreeFlags::WBT_DELTA_NUMERATOR>() &&
		              (utilities::get_value_if_present<
		                   TreeFlags::WBT_GAMMA_DENOMINATOR, Opts...>::value !=
		               0)) {
			return (utilities::get_value_if_present<TreeFlags::WBT_GAMMA_NUMERATOR,
			                                        Opts...>::value /
			        utilities::get_value_if_present<TreeFlags::WBT_GAMMA_DENOMINATOR,
			                                        Opts...>::value);
		} else {
			return wbt_gamma_default;
		}
	}

	static std::string
	wbt_gamma_str()
	{
		if constexpr (OptPack::template has_tmpl_size_t<
		                  TreeFlags::WBT_DELTA_DENOMINATOR>() &&
		              OptPack::template has_tmpl_size_t<
		                  TreeFlags::WBT_DELTA_NUMERATOR>() &&
		              (utilities::get_value_if_present<
		                   TreeFlags::WBT_GAMMA_DENOMINATOR, Opts...>::value !=
		               0)) {
			if (utilities::get_value_if_present<TreeFlags::WBT_GAMMA_DENOMINATOR,
			                                    Opts...>::value != 1) {
				return std::to_string(utilities::get_value_if_present<
				                      TreeFlags::WBT_GAMMA_NUMERATOR, Opts...>::value) +
				       std::string("/") +
				       std::to_string(
				           utilities::get_value_if_present<
				               TreeFlags::WBT_GAMMA_DENOMINATOR, Opts...>::value);
			} else {
				return std::to_string(
				    utilities::get_value_if_present<TreeFlags::WBT_GAMMA_NUMERATOR,
				                                    Opts...>::value);
			}
		} else {
			return std::string("sqrt(2)");
		}
	}

	static constexpr bool wbt_single_pass =
	    OptPack::template has<TreeFlags::WBT_SINGLE_PASS>();

	/**********************************************
	 * Micro-Optimization
	 **********************************************/
	static constexpr bool micro_avoid_conditionals =
	    OptPack::template has<TreeFlags::MICRO_AVOID_CONDITIONALS>();

	/**********************************************
	 * Benchmarking Helpers
	 **********************************************/
	using SequenceInterface =
	    typename decltype(compute_sequence_interface_type())::type;
	/// @endcond
private:
	TreeOptions(); // Instantiation not allowed
};

using DefaultOptions =
    TreeOptions<TreeFlags::MULTIPLE, TreeFlags::CONSTANT_TIME_SIZE>;

} // namespace ygg

#endif // YGG_OPTIONS_HPP
