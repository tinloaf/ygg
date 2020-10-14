#ifndef YGG_NODEPOOL_CPP
#define YGG_NODEPOOL_CPP

#include "nodepool.hpp"

namespace ygg {
namespace bst {
namespace nodepool {

template <class BST>
void
NodePool<BST>::insert(Node * n) noexcept
{
	n->NodeBase::_bst_children[0] = this->root;
	this->root = n;
}

template <class BST>
typename BST::MyNode *
NodePool<BST>::get() noexcept
{
	auto ret = this->root;
	if (ret != nullptr) {
		this->root = ret->NodeBase::_bst_children[0];
	}
	return ret;
}

} // namespace nodepool
} // namespace bst
} // namespace ygg

#endif
