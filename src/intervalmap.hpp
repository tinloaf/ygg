//
// Created by lukas on 31.08.17.
//

#ifndef YGG_INTERVALMAP_HPP
#define YGG_INTERVALMAP_HPP

#include "intervaltree.hpp"
#include "rbtree.hpp"

namespace ygg {

namespace internal {
	template<class KeyT, class ValueT, int Tag>
	class InnerNode : public  RBTreeNodeBase<InnerNode<KeyT, ValueT, Tag>,
	                                                   TreeOptions<TreeFlags::MULTIPLE>, Tag> {
	public:
		KeyT point;
		ValueT aggregate;

		bool operator<(const InnerNode<KeyT, ValueT, Tag> & rhs) {
			return this->point < rhs.point;
		}
	};
} // namespace internal


template<class KeyT, class ValueT, int Tag = 0>
class IMapNodeBase
{
public:
	using Segment = internal::InnerNode<KeyT, ValueT, Tag>;
	Segment _imap_begin;
	Segment _imap_end;

	using value_type = ValueT;
	using key_type = ValueT;
};

template<class Node>
class IMapNodeTraits {
public:
	using key_type = typename Node::key_type;
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

	// TODO DOC
	static value_type  get_value(const Node & n) = delete;

	// TODO DOC
	static void on_value_change(typename Node::Segment & seg, const value_type & old_val,
	                            const value_type & new_val) {} ;
};


template <class Node, class NodeTraits, int Tag = 0>
class IntervalMap {
public:
	static_assert(std::is_base_of<IMapNodeTraits<Node>, NodeTraits>::value,
	              "NodeTraits not properly derived from IMapNodeTraits!");

	using NB = IMapNodeBase<typename Node::key_type, typename Node::value_type, Tag>;
	static_assert(std::is_base_of<NB, Node>::value,
	              "Node class not properly derived from IMapNodeBase!");
	using Segment = internal::InnerNode<typename Node::key_type, typename Node::value_type, Tag>;
	using ITree = RBTree<Segment, RBDefaultNodeTraits<Segment>, TreeOptions<TreeFlags::MULTIPLE>,
	                     Tag>;

	void insert(Node & n);
	void remove(Node & n);

	using value_type = typename Node::value_type;
	using key_type = typename Node::key_type;

	value_type get_aggregate(Segment & n);

	class iterator {
	public:
		/// @cond INTERNAL
		using InnerIterator = typename ITree::template const_iterator<false>;

		typedef typename InnerIterator::difference_type      difference_type;
		typedef typename InnerIterator::value_type           value_type;
		typedef typename InnerIterator::const_reference      const_reference;
		typedef typename InnerIterator::const_pointer        const_pointer;
		typedef std::input_iterator_tag             iterator_category;

		iterator ();
		iterator (const InnerIterator & it, const ITree * t);
		iterator (const iterator & other);

		iterator& operator=(const iterator & other);
		iterator& operator=(iterator && other);

		bool operator==(const iterator & other) const;
		bool operator!=(const iterator & other) const;

		iterator& operator++();
		iterator  operator++(int);
		iterator& operator+=(size_t steps);
		iterator  operator+(size_t steps) const;

		iterator& operator--();
		iterator  operator--(int);

		const_reference operator*() const;
		const_pointer operator->() const;
		/// @endcond

		key_type get_lower() const;
		key_type get_upper() const;
		const typename IntervalMap<Node, NodeTraits, Tag>::value_type & get_value() const;

	private:
		/// @cond INTERNAL

		// TODO store the next pointer!
		InnerIterator inner;
		const ITree * t;

		/// @endcond
	};

	iterator begin() const;
	iterator end() const;

private:
	ITree t;


};

} // namespace ygg

#include "intervalmap.cpp"

#endif // YGG_INTERVALMAP_HPP

