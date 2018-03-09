//
// Created by lukas on 07.02.18.
//

#ifndef YGG_RBTREE_ATTRIBUTES_HPP
#define YGG_RBTREE_ATTRIBUTES_HPP

namespace ygg {
namespace rbtree_attrs {

template<class Node, class KeyT, class KeyGetter>
class RangeAttribute {
public:
	RangeAttribute();
	bool rebuild(Node * self, RangeAttribute<Node, KeyT, KeyGetter> * left,
	             RangeAttribute<Node, KeyT, KeyGetter> * right);

private:
	KeyT lower;
	KeyT upper;
};

} // namespace rbtree_attrs
} // namespace ygg

#endif //YGG_RBTREE_ATTRIBUTES_HPP
