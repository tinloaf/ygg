//
// Created by lukas on 18.09.17.
//

#ifndef YGG_LIST_HPP
#define YGG_LIST_HPP

#include <type_traits>
#include <cstddef>
#include <iterator>

#include "options.hpp"
#include "size_holder.hpp"


namespace ygg {

/**
 * @brief Base class (template) to supply your node class with metainformation
 *
 * The class you use as nodes for the linked list *must* derive from this
 * class (template). It supplies your class with the necessary members to
 * contain the linking between the list nodes.
 *
 * @tparam Node    The node class itself. Yes, that's the class derived from this template. This
 * sounds weird, but is correct. See the examples if you're confused.
 * @tparam Tag 		The tag used to identify the tree that this node should be inserted into. See
 * List for details.
 */
template<class Node, class Tag = int>
class ListNodeBase {
public:
	Node * _l_prev;
	Node * _l_next;
};

/**
 * @brief An intrusive doubly-linked list class
 *
 * This class serves as an intrusive doubly-linked list on the nodes.
 *
 * @tparam Node 	The class of the nodes is this list. Must be derived from ListNodeBase.
 * @tparam Tag		An class tag that identifies this list. Can be used to insert the same
 * nodes into multiple lists. See DOCTODO for details. Can be any class, the class can be empty.
 * Must be the same tag as specified for the nodes.
 */
template<class Node, class Options = DefaultOptions, class Tag = int>
class List {
public:
	using NB = ListNodeBase<Node, Tag>;

	static_assert(std::is_base_of<ListNodeBase<Node, Tag>, Node>::value,
	              "Node class not properly derived from ListNodeBase");

	/// @cond INTERNAL
	template<class ConcreteIterator, class BaseType>
	class IteratorBase {
	public:
		typedef ptrdiff_t                         difference_type;
		typedef BaseType                          value_type;
		typedef BaseType &                        reference;
		typedef BaseType *                        pointer;
		typedef std::input_iterator_tag           iterator_category;

		IteratorBase ();
		IteratorBase (List<Node, Options, Tag> * l, BaseType * n);
		IteratorBase (const ConcreteIterator & other);

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
		ConcreteIterator& operator-=(size_t steps);
		ConcreteIterator  operator-(size_t steps) const;

		reference operator*() const;
		pointer operator->() const;

	protected:
		BaseType * n;
		List<Node, Options, Tag> * l;
	};
	/// @endcond

	class const_iterator; // forward for friendship

	/**
	 * @brief Iterator over all elements in the list
	 */
	class iterator : public IteratorBase<iterator, Node> {
	public:
		using IteratorBase<iterator, Node>::IteratorBase;
		friend  class const_iterator;
	};


	/**
	 * @brief Const iterator over all elements in the list
	 */
	class const_iterator : public IteratorBase<const_iterator, const Node> {
	public:
		using IteratorBase<const_iterator, const Node>::IteratorBase;
		const_iterator(const iterator & other) :
						IteratorBase<const_iterator, const Node>::IteratorBase(other.l, other.n) {};
	};

	/**
	 * Constructs an empty list.
	 */
	List();

	/**
	 * @brief Insert a node into the list
	 *
	 * This inserts Node n into the list, right before next. To insert a node at the end of the
	 * list, set next to nullptr.
	 *
	 * @param next 	The node that n should be inserted before. Set to nullptr to insert n at the
	 * end of the list.
	 * @param n 		The node to be inserted into the list.
	 */
	void insert(Node *next, Node *n);

	/**
	 * @brief Remove a node from the list.
	 *
	 * @param n 	The node to be removed from the list.
	 */
	void remove(Node *n);

	/**
	 * Returns an iterator pointing to the first element in the list.
	 *
	 * @return An iterator pointing to the first element in the list.
	 */
	iterator begin();
	const_iterator begin() const;

	/**
	 * Returns an iterator pointing after the last element in the list.
	 *
	 * @return An iterator pointing after the last element in the list.
	 */
	iterator end();
	const_iterator end() const;

	/**
	 * Returns an iterator pointing to the last element in the list.
	 *
	 * @return An iterator pointing to the last element in the list.
	 */
	iterator back();
	const_iterator back() const;

	/**
	 * Returns an iterator pointing to n in the list
	 *
	 * @param n		The node to which a pointer should be returned.
	 * @return 		An iterator pointing to n in the list.
	 */
	const_iterator iterator_to(const Node & n) const;
	iterator iterator_to(const Node & n);

	/**
	 * @brief Returns the number of elements in the list
	 *
	 * This method runs in O(1).
	 *
	 * @warning This method is only available if CONSTANT_TIME_SIZE is set.
	 *
	 * @return 	The number of elements in the list
	 */
	size_t size() const;

private:
	Node * head;
	Node * tail;

	SizeHolder<Options::constant_time_size> s;
};

} // namespace ygg

#include "list.cpp"

#endif //YGG_LIST_HPP
