//
// Created by lukas on 31.08.17.
//

#ifndef YGG_INTERVALMAP_HPP
#define YGG_INTERVALMAP_HPP

#include "intervaltree.hpp"
#include "rbtree.hpp"
#include "list.hpp"
#include "options.hpp"
#include "size_holder.hpp"

namespace ygg {

namespace intervalmap_internal {
	/// @cond INTERNAL
	class SegListTag {};
	class RepresentativeSegListTag {};
	class InnerRBTTag {};

	template<class KeyT, class ValueT>
	class InnerNode : public RBTreeNodeBase<InnerNode<KeyT, ValueT>,
	                                                   TreeOptions<TreeFlags::MULTIPLE>, InnerRBTTag>,
										public ListNodeBase<InnerNode<KeyT, ValueT>, SegListTag>,
										public ListNodeBase<InnerNode<KeyT, ValueT>, RepresentativeSegListTag>
	{
	public:
		KeyT point;
		ValueT aggregate;
		InnerNode<KeyT, ValueT> * repr;

		class Compare {
		public:
			constexpr bool operator()(const InnerNode<KeyT, ValueT> & lhs,
			                          const InnerNode<KeyT, ValueT> & rhs) const
			{
				return lhs.point < rhs.point;
			}

			constexpr bool operator()(int lhs,
			                          const InnerNode<KeyT, ValueT> & rhs) const
			{
				return lhs < rhs.point;
			}

			constexpr bool operator()(const InnerNode<KeyT, ValueT> & lhs,
			                          int rhs) const
			{
				return lhs.point < rhs;
			}
		};
	};
	/// @endcond
} // namespace intervalmap_internal

/**
 * @brief Base class (template) to supply your node class with metainformation for inclusion in
 * an IntervalMap
 *
 * The class you use as node within the ItervalMap *must* derive from this class template. I
 * supplies your node class with the necessary members to contain the metainformation for
 * managing the underlying RBTree.
 *
 * See the IntervalMap for more documentation on keys, values, tags and how the IntervalMap behaves.
 *
 * @tparam KeyT			The type of the keys in the IntervalMap.
 * @tparam ValueT		The type of the values in the IntervalMap. Must be default-constructible,
 * addable and subtractable.
 * @tparam Tag			The tag used to identify the underlying RBTree. If you want your nodes to be
 * part of multiple IntervalMaps, RBTrees or IntervalTrees, each must have its own unique tag.
 * Can be any class, the class can be empty.
 */
template<class KeyT, class ValueT, class Tag = int>
class IMapNodeBase
{
public:
	/**
	 * Inserting nodes into an IntervalMap results in the key space to be divided into Segments.
	 * See DOCTODO for details and examples. This is the type that these segments will have.
	 */
	using Segment = intervalmap_internal::InnerNode<KeyT, ValueT>;

	/// @cond INTERNAL
	Segment _imap_begin;
	Segment _imap_end;

	using value_type = ValueT;
	using key_type = KeyT;
	/// @endcond
};

/**
 * @brief You must derive your own traits class from this class template, telling the IntervalMap
 * how to interact with your node class.
 *
 * You must derive from this class template and supply the IntervalMap with your own derived
 * class. At the least, you have to implement the methods get_lower, get_upper and get_value for
 * the IntervalMap to work. See the respective methods' documentation for details.
 *
 * @tparam Node 	Your node class to be used in the IntervalMap
 */
template<class Node>
class IMapNodeTraits {
public:
	/**
	 * The type of the keys of intervals / segments in the IntervalMap
	 */
	using key_type = typename Node::key_type;
	/**
	 * The type of the values / aggregates in the IntervalMap
	 */
	using value_type = typename Node::value_type;

	/**
	 * Must be implemented to return the lower bound of the interval represented by n.
	 *
	 * @param n The node whose lower interval bound should be returned.
	 * @return Must return the lower interval bound of n
	 */
	static key_type get_lower(const Node & n) = delete;

	/**
	 * Must be implemented to return the upper bound of the interval represented by n.
	 *
	 * @param n The node whose upper interval bound should be returned.
	 * @return Must return the upper interval bound of n
	 */
	static key_type get_upper(const Node & n) = delete;

	/**
	 * Must be implemented to return the value associated with the interval represented by n.
	 *
	 * @param n The node whose associated value should be returned
	 * @return Must return the value associated with n
	 */
	static value_type get_value(const Node & n) = delete;

	/**
	 * Callback that is called when the aggregate value for a segment changes. See DOCTODO for
	 * information on segments.
	 *
	 * @param seg 		The segment that changed
	 * @param old_val The old aggregate value associated with the segment
	 * @param new_val The new aggregate value associated with the segment
	 */
	static void on_value_changed(typename Node::Segment & seg, const value_type & old_val,
	                             const value_type & new_val) {
		(void)seg;
		(void)old_val;
		(void)new_val;
	}

	/**
	 * Callback that is called when the length of a segment changes. See TODO for how to retrieve
	 * the length of a segment; especially note that there are zero-length segments. See DOCTODO
	 * for information on segments.
	 *
	 * Note that this will also be called on newly inserted segments; the aggregate value for those
	 * segments will then not be updated yet.
	 *
	 * @param seg The segment that changed its length.
	 */
	static void on_length_changed(typename Node::Segment & seg)
	{
		(void) seg;
	}

	/**
	 * Callback that is called when a new segment was created. Note that at the point this callback
	 * is called, the aggregate value associated with the segment is not yet determined. See DOCTODO
	 * for information on segments.
	 *
	 * @param seg The newly inserted segment
	 */
	static void on_segment_inserted(typename Node::Segment & seg)
	{
		(void) seg;
	}

	/**
	 * Callback that is called when a segment is destroyed. See DOCTODO for information on segments.
	 *
	 * @param seg The removed segment
	 */
	static void on_segment_removed(typename Node::Segment & seg)
	{
		(void) seg;
	}
};

/**
 * @brief An IntervalMap stores a collection of intervals that are associated with a value and
 * allows for efficient access to aggregate values
 *
 * An IntervalMap stores a collection of intervals that are associated with a value. Where
 * multiple intervals in the interval map overlap, values are aggregated (e.g., by adding them
 * up). The interval map then allows to efficiently query for the aggregated value at a certain
 * point, to iterate the whole "horizon" of intervals and their respective values, and so on.
 *
 * To this end, the whole "horizon", that is, the space between the smallest lower interval
 * border and the largest upper interval border in the map, is divided into segments. A segment
 * represents a contiuous interval (note: *not* necessarily one of the intervals inserted
 * into the map!) during which the aggregate value does not change. Note that with n intervals
 * inserted into the map, there are at most 2n - 1 such segments. In fact, for implementation
 * reasons, there will always be exactly 2n - 1 segments in an IntervalMap. However, where
 * multiple intervals start or end at the same point, segments of length 0 occurr.
 *
 * @tparam Node					The node class for the interval map. Must be derived from IMapNodeBase.
 * @tparam NodeTraits		The node traits, mainly defining how the IntervalMap retrieves data from
 * your nodes. Must be derived from IMapNodeTraits.
 * @tparam Options			The TreeOptions class specifying the parameters of this IntervalMap. See the
 * TreeOptions and TreeFlags classes for details.
 * @tparam Tag					The tag used to identify the underlying RBTree. If you want your nodes to be
 * part of multiple IntervalMaps, RBTrees or IntervalTrees, each must have its own unique tag.
 * Can be any class, the class can be empty.
 */
template <class Node, class NodeTraits, class Options = DefaultOptions, class Tag = int>
class IntervalMap {
public:
	/**
 * @brief A segment is a continuous range during which the aggregated value does not change.
 *
 * Not that at the start and end of every segment, at least one interval starts or ends.
 * However, not all interval starts and ends create a segment border, because two consecutive
 * intervals might have the same value, thus resulting in a single segment.
 *
 * @note Internally, the IntervalMap contains more segments than described above. However, the
 * IntervalMap API presents the segments as described above.
 */
	using Segment = intervalmap_internal::InnerNode<typename Node::key_type, typename Node::value_type>;

	/// @cond internal
	static_assert(std::is_base_of<IMapNodeTraits<Node>, NodeTraits>::value,
	              "NodeTraits not properly derived from IMapNodeTraits!");

	static_assert(Options::multiple, "IntervalMap always allows multiple equal intervals.");

	using NB = IMapNodeBase<typename Node::key_type, typename Node::value_type, Tag>;
	static_assert(std::is_base_of<NB, Node>::value,
	              "Node class not properly derived from IMapNodeBase!");
	using ITree = RBTree<Segment, RBDefaultNodeTraits<Segment>, TreeOptions<TreeFlags::MULTIPLE>,
	                     intervalmap_internal::InnerRBTTag, typename Segment::Compare>;
	using SegList = List<Segment, TreeOptions<>, intervalmap_internal::SegListTag>;
	using RepresentativeSegList = List<Segment, TreeOptions<>, intervalmap_internal::RepresentativeSegListTag>;
	using value_type = typename Node::value_type;
	using key_type = typename Node::key_type;
	/// @endcond

	/**
	 * @brief Inserts a node into the IntervalMap.
	 *
	 * TODO iterator invalidation
	 *
	 * @param n The node to be inserted.
	 */
	void insert(Node & n);

	/**
	 * @brief Removes a node from the IntervalMap.
	 *
	 * TODO iterator invalidation
	 *
	 * @param n The node to be removed.
	 */
	void remove(Node & n);

	/**
	 * @brief Returns the number of intervals currently in the map
	 *
	 * This method runs in O(1).
	 *
	 * @warning This method is only available if CONSTANT_TIME_SIZE is set.
	 *
	 * @return 	The number of intervals in the IntervalMap
	 */
	size_t size() const;

	/**
	 * @brief Returns whether the interval map is empty
	 *
	 * This method runs in O(1).
	 *
	 * @return true if the interval map is empty, false otherwise
	 */
	 bool empty() const;

	/**
	 * @brief Returns the aggregate value during a segment
	 *
	 * @param s 	The segment the aggregate value of which should be returned
	 * @return 		The aggregate value during s
	 */
	value_type get_aggregate(Segment & s);

	/// @cond INTERNAL
	template<class ConcreteIterator, class InnerIterator>
	class IteratorBase {
	public:


		typedef typename InnerIterator::difference_type      difference_type;
		typedef typename InnerIterator::value_type           value_type;
		typedef typename InnerIterator::reference            reference;
		typedef typename InnerIterator::pointer              pointer;
		typedef std::input_iterator_tag             iterator_category;

		IteratorBase();
		IteratorBase(const InnerIterator & it, RepresentativeSegList * l);
		IteratorBase(const ConcreteIterator & other);

		ConcreteIterator& operator=(const ConcreteIterator & other);
		ConcreteIterator& operator=(ConcreteIterator && other);

		bool operator==(const ConcreteIterator & other) const;
		bool operator!=(const ConcreteIterator & other) const;

		ConcreteIterator& operator++();
		ConcreteIterator  operator++(int);
		ConcreteIterator& operator+=(size_t steps);
		ConcreteIterator  operator+(size_t steps) const;

		ConcreteIterator& operator--();
		ConcreteIterator  operator--(int);

		reference operator*() const;
		pointer operator->() const;

		key_type get_lower() const;
		key_type get_upper() const;
		const typename IntervalMap<Node, NodeTraits, Options, Tag>::value_type & get_value() const;
	private:

		InnerIterator inner;
		RepresentativeSegList * l;
	};
	/// @cond INTERNAL

	/**
	 * @brief A constant iterator over all Segment s in the map
	 */
	class const_iterator : public IteratorBase<const_iterator, typename RepresentativeSegList::const_iterator> {
	public:
		using IteratorBase<const_iterator, typename RepresentativeSegList::const_iterator>::IteratorBase;
	};

	/**
	 * @brief An iterator over all Segment s in the map
	 */
	class iterator : public IteratorBase<iterator, typename RepresentativeSegList::iterator> {
	public:
		using IteratorBase<iterator, typename RepresentativeSegList::iterator>::IteratorBase;
	};

	const_iterator begin() const;
	const_iterator end() const;
	iterator begin();
	iterator end();

	void dbg_verify();

private:
	Segment * get_head(Segment * seg);

	Segment * insert_segment(Segment * seg);
	void remove_segment(Segment * seg);

	void repr_now_equal(Segment *a, Segment *b);
	void repr_now_different(Segment *a, Segment *b);
	void repr_replaced(Segment *old, Segment *replacement);

	// TODO FIXME is this needed?
	iterator find_lower_bound_representative(typename Node::key_type point);

	ITree t;
	SegList l;
	RepresentativeSegList repr_list;

	SizeHolder<Options::constant_time_size> s;

	void dbg_verify_list();
	void dbg_verify_representatives();
};

} // namespace ygg

#include "intervalmap.cpp"

#endif // YGG_INTERVALMAP_HPP

