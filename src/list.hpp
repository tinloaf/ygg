//
// Created by lukas on 18.09.17.
//

#ifndef YGG_LIST_HPP
#define YGG_LIST_HPP

#include <type_traits>
#include <cstddef>
#include <iterator>

namespace ygg {

template<class Node, int Tag = 0>
class ListNodeBase {
public:
	Node * _l_prev;
	Node * _l_next;
};

template<class Node, int Tag = 0>
class List {
public:
	using NB = ListNodeBase<Node, Tag>;

	template<class ConcreteIterator, class BaseType>
	class IteratorBase {
	public:
		/*static_assert(std::is_base_of<IteratorBase<ConcreteIterator, BaseType>,
		                              ConcreteIterator>::value,
		              "Node class not properly derived from ListNodeBase");*/

		/// @cond INTERNAL
		typedef ptrdiff_t                         difference_type;
		typedef BaseType                          value_type;
		typedef BaseType &                        reference;
		typedef BaseType *                        pointer;
		typedef std::input_iterator_tag           iterator_category;

		IteratorBase ();
		IteratorBase (List * l, BaseType * n);
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

		reference operator*() const;
		pointer operator->() const;

	protected:
		BaseType * n;
		List<Node, Tag> * l;
	};

	class iterator : public IteratorBase<iterator, Node> {
	public:
		using IteratorBase<iterator, Node>::IteratorBase;
	};

	class const_iterator : public IteratorBase<const iterator, const Node> {
	public:
		using IteratorBase<const_iterator, const Node>::IteratorBase;
	};

	List();

	void insert(Node *next, Node *n);
	void remove(Node *n);

	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;

	const_iterator iterator_to(const Node & n) const;

private:
	Node * head;
	Node * tail;
};

} // namespace ygg

#include "list.cpp"

#endif //YGG_LIST_HPP
