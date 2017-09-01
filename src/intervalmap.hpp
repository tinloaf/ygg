//
// Created by lukas on 31.08.17.
//

#ifndef YGG_INTERVALMAP_HPP
#define YGG_INTERVALMAP_HPP

#include "intervaltree.hpp"
#include "rbtree.hpp"

namespace ygg {

namespace internal {

	class Constants {
	public:
		static constexpr const int IT_BEGIN  = 1;
		static constexpr const int IT_END    = 2;
		static constexpr const int RBT_LEVEL = 3;
	};

	template<class NodeT, class KeyT>
	class IntervalNodeTraits : public ITreeNodeTraits<Node> {
		using key_type = KeyT;

		static key_type get_lower(const Node & node);
		static key_type get_upper(const Node & node);
	};

	template<class Node, class KeyT>
	using BeginNodeBase = ITreeNodeBase<Node, IntervalNodeTraits<Node, KeyT>,
	                                    TreeOptions<TreeFlags::MULTIPLE>, Constants::IT_BEGIN>;
	template<class Node, class KeyT>
	using EndNodeBase = ITreeNodeBase<Node, IntervalNodeTraits<Node, KeyT>,
	                                    TreeOptions<TreeFlags::MULTIPLE>, Constants::IT_END>;
	template<class Node>
	using LevelNodeBase = RBTreeNodeBase<Node, TreeOptions<TreeFlags::MULTIPLE>,
	                                     Constants::RBT_LEVEL>;

	template<class KeyT>
	class Node : public BeginNodeBase<Node, KeyT>, public EndNodeBase<Node, KeyT>,
	             public LevelNodeBase <Node>
	{};
} // namespace internal

template<class ValueType>
class IntervalBase {
	using key_type = void;

	key_type get_lower() = delete;
	key_type get_upper() = delete;

	ValueType get_value() = delete;
};

template <class Interval, class ValueType>
class IntervalMap {
public:
	IntervalMap(unsigned int n_elements);

private:
	using ITree = IntervalTree<
};

} // namespace ygg

#endif // YGG_INTERVALMAP_HPP
