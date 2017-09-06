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


		class Compare {
		public:
			constexpr bool operator()(const InnerNode<KeyT, ValueT, Tag> & lhs,
			                          const InnerNode<KeyT, ValueT, Tag> & rhs) const
			{
				return lhs.point < rhs.point;
			}

			constexpr bool operator()(int lhs,
			                          const InnerNode<KeyT, ValueT, Tag> & rhs) const
			{
				return lhs < rhs.point;
			}

			constexpr bool operator()(const InnerNode<KeyT, ValueT, Tag> & lhs,
			                          int rhs) const
			{
				return lhs.point < rhs;
			}
		};
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
	static void on_value_changed(typename Node::Segment & seg, const value_type & old_val,
	                             const value_type & new_val) {
		(void)seg;
		(void)old_val;
		(void)new_val;
	} ;
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
	                     Tag, typename Segment::Compare>;

	void insert(Node & n);
	void remove(Node & n);

	using value_type = typename Node::value_type;
	using key_type = typename Node::key_type;

	value_type get_aggregate(Segment & n);

	template<class ConcreteIterator, class InnerIterator>
	class IteratorBase {
	public:
		/// @cond INTERNAL


		typedef typename InnerIterator::difference_type      difference_type;
		typedef typename InnerIterator::value_type           value_type;
		typedef typename InnerIterator::reference            reference;
		typedef typename InnerIterator::pointer              pointer;
		typedef std::input_iterator_tag             iterator_category;

		IteratorBase();
		IteratorBase(const InnerIterator & it, ITree * t);
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
		/// @endcond

		key_type get_lower() const;
		key_type get_upper() const;
		const typename IntervalMap<Node, NodeTraits, Tag>::value_type & get_value() const;

	private:
		/// @cond INTERNAL

		// TODO store the next pointer!
		InnerIterator inner;
		ITree * t;

		/// @endcond
	};

	class const_iterator : public IteratorBase<const_iterator,
	                                           typename ITree::template const_iterator<false>> {
	public:
		using IteratorBase<const_iterator, typename ITree::template const_iterator<false>>::IteratorBase;
	};

	class iterator : public IteratorBase<iterator, typename ITree::template iterator<false>> {
	public:
		using IteratorBase<iterator, typename ITree::template iterator<false>>::IteratorBase;
	};

	const_iterator begin() const;
	const_iterator end() const;
	iterator begin();
	iterator end();

private:
	ITree t;


};

} // namespace ygg

#include "intervalmap.cpp"

#endif // YGG_INTERVALMAP_HPP

