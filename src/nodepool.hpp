#ifndef YGG_NODEPOOL_HPP
#define YGG_NODEPOOL_HPP

#include "bst.hpp"

namespace ygg {
namespace bst {
namespace nodepool {

template <class BST>
class NodePool {
private:
	using NodeBase = typename BST::NB;
	using Node = typename BST::MyNode;
	Node * root = nullptr;

public:
	void insert(Node * n) noexcept;
	Node * get() noexcept;
};

} // namespace nodepool
} // namespace bst
} // namespace ygg

#ifndef YGG_NODEPOOL_CPP
#include "nodepool.cpp"
#endif

#endif
