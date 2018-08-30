#ifndef YGG_ZIPTREE_CPP
#define YGG_ZIPTREE_CPP

#include "ziptree.hpp"

#include <vector>
#include <fstream>
#include <iostream>

namespace ygg {

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::ZTree() noexcept
    : root(nullptr), cmp()
{}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::insert(
    Node & node) noexcept
{
  // First, search for insertion position.
  auto node_rank = RankGetter::get_rank(node);
  this->s.add(1);

  // TODO this should be handled by the code below
  if (this->root == nullptr) {
    this->root = &node;
    return;
  }

  Node * current = this->root;
  Node * last = nullptr;
  bool last_left = false;

  while ((current != nullptr) &&
         ((node_rank < RankGetter::get_rank(*current)) ||
          ((node_rank == RankGetter::get_rank(*current)) &&
           (!this->cmp(*current, node))))) {
    last = current;
    // On equality, go left
    if (this->cmp(*current, node)) {
      last_left = false;
      current = current->_zt_right;
    } else {
      last_left = true;
      current = current->_zt_left;
    }
  }

  // Place new node in place of old
  if (last != nullptr) {
    node._zt_parent = last;
    if (last_left) {
      last->_zt_left = &node;
    } else {
      last->_zt_right = &node;
    }
  } else {
    // We replace the root
    node._zt_parent = nullptr;
    this->root = &node;
  }

  if (current != nullptr) {
    this->unzip(*current, node);
  }
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare,
      RankGetter>::dbg_print_rank_stats() const
{
  std::vector<size_t> rank_count;

  for (auto & node : *this) {
    size_t rank = RankGetter::get_rank(node);
    if (rank_count.size() <= rank) {
      rank_count.resize(rank + 1, 0);
    }
    rank_count[rank]++;
  }

  for (unsigned int rank = 1 ; rank < rank_count.size() ; ++rank) {
    std::cout << "Rank " << rank << "\t: " << rank_count[rank] << std::endl;
  }
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::unzip(
    Node & oldn, Node & newn) noexcept
{
  Node * left_head = &newn;
  Node * right_head = &newn;

  Node * cur = &oldn;
  while (cur != nullptr) {
    if (this->cmp(newn, *cur)) {
      // Add to the right spine

      if (right_head != &newn) {
	right_head->_zt_left = cur;
      } else {
	right_head->_zt_right = cur;
      }

      cur->_zt_parent = right_head;
      right_head = cur;

      cur = cur->_zt_left;
    } else {
      // Add to the left spine

      if (left_head != &newn) {
	left_head->_zt_right = cur;
      } else {
	left_head->_zt_left = cur;
      }

      cur->_zt_parent = left_head;
      left_head = cur;

      cur = cur->_zt_right;
    }
  }

  // End of the spines
  if (left_head != &newn) {
    left_head->_zt_right = nullptr;
  } else {
    left_head->_zt_left = nullptr;
  }

  if (right_head != &newn) {
    right_head->_zt_left = nullptr;
  } else {
    right_head->_zt_right = nullptr;
  }
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::remove(
    Node & n) noexcept
{
  this->s.reduce(1);
  this->zip(n);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::zip(
    Node & old_root) noexcept
{
  Node * left_head = old_root._zt_left;
  Node * right_head = old_root._zt_right;

  Node * cur = old_root._zt_parent;

  bool last_from_left;

  // First one is special.
  // TODO simplify this code!
  if ((left_head != nullptr) && (right_head != nullptr)) {
    // Both are there, use the one with the larger rank
    if (RankGetter::get_rank(*left_head) > RankGetter::get_rank(*right_head)) {
      last_from_left = true;

      if (cur == nullptr) {
	this->root = left_head;
	left_head->_zt_parent = nullptr;
      } else {
	if (cur->_zt_left == &old_root) {
	  cur->_zt_left = left_head;
	} else {
	  assert(cur->_zt_right == &old_root);
	  cur->_zt_right = left_head;
	}
	left_head->_zt_parent = cur;
      }
      cur = left_head;
      left_head = left_head->_zt_right;
    } else {
      last_from_left = false;

      if (cur == nullptr) {
	this->root = right_head;
	right_head->_zt_parent = nullptr;
      } else {
	if (cur->_zt_left == &old_root) {
	  cur->_zt_left = right_head;
	} else {
	  assert(cur->_zt_right == &old_root);
	  cur->_zt_right = right_head;
	}

	right_head->_zt_parent = cur;
      }
      cur = right_head;
      right_head = right_head->_zt_left;
    }
  } else if (left_head != nullptr) {
    // Only left is there, use it!
    last_from_left = true;

    if (cur == nullptr) {
      this->root = left_head;
      left_head->_zt_parent = nullptr;
    } else {
      if (cur->_zt_left == &old_root) {
	cur->_zt_left = left_head;
      } else {
	assert(cur->_zt_right == &old_root);
	cur->_zt_right = left_head;
      }
      left_head->_zt_parent = cur;
    }
    cur = left_head;
    left_head = left_head->_zt_right;
  } else if (right_head != nullptr) {
    // Only right is there, use it.
    last_from_left = false;

    if (cur == nullptr) {
      this->root = right_head;
      right_head->_zt_parent = nullptr;
    } else {
      if (cur->_zt_left == &old_root) {
	cur->_zt_left = right_head;
      } else {
	assert(cur->_zt_right == &old_root);
	cur->_zt_right = right_head;
      }

      right_head->_zt_parent = cur;
    }

    cur = right_head;
    right_head = right_head->_zt_left;
  } else {
    // Both child-trees are empty. This is a special case: Just remove the node
    // and return, no zipping necessary.
    if (cur == nullptr) {
      this->root = nullptr;
    } else {
      if (cur->_zt_left == &old_root) {
	cur->_zt_left = nullptr;
      } else {
	assert(cur->_zt_right == &old_root);
	cur->_zt_right = nullptr;
      }
    }
    return;
  }

  // Now, walk down left and right
  // TODO as soon as one becomes nullptr, we can just rehang the whole subtree,
  // no need for iteration!
  while ((left_head != nullptr) || (right_head != nullptr)) {
    if ((right_head == nullptr) ||
        ((left_head != nullptr) && (RankGetter::get_rank(*left_head) >
                                    RankGetter::get_rank(*right_head)))) {
      // Use left

      if (last_from_left) {
	// just pass on
      } else {
	// last was from the right, we can just use its left child.
	cur->_zt_left = left_head;
	left_head->_zt_parent = cur;
      }

      cur = left_head;
      /*
      left_head = (left_head->_zt_right != nullptr) ? left_head->_zt_right
                                                    : left_head->_zt_left;
      */
      left_head = left_head->_zt_right;
      last_from_left = true;
    } else {
      // use right
      if (!last_from_left) {
	// just pass on
      } else {
	// last was from the left, we can just use its right child.
	cur->_zt_right = right_head;
	right_head->_zt_parent = cur;
      }
      cur = right_head;
      /*
      right_head = (right_head->_zt_left != nullptr) ? right_head->_zt_left
                                                     : right_head->_zt_right;
      */
      right_head = right_head->_zt_left;
      last_from_left = false;
    }
  }
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::dbg_verify() const
{
  this->dbg_verify_consistency(this->root, nullptr, nullptr);
  if (Options::constant_time_size) {
    this->dbg_verify_size();
  }
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::dbg_verify_size()
    const
{
  size_t node_count = 0;
  for (auto & node : *this) {
    (void)node;
    node_count++;
  }

  ztree_internal::dbg_verify_size_helper<my_type,
                                         Options::constant_time_size>{}(
      *this, node_count);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare,
      RankGetter>::dbg_verify_consistency(Node * sub_root, Node * lower_bound,
                                          Node * upper_bound) const
{
  if (sub_root == nullptr) {
    return;
  }

  if (sub_root->_zt_parent == nullptr) {
    assert(this->root == sub_root);
  } else {
    assert(this->root != sub_root);
  }

  if (lower_bound != nullptr) {
    assert(this->cmp(*lower_bound, *sub_root));
  }
  if (upper_bound != nullptr) {
    assert(!this->cmp(*upper_bound, *sub_root));
  }

  if (sub_root->_zt_right != nullptr) {
    assert(RankGetter::get_rank(*sub_root->_zt_right) <
           RankGetter::get_rank(*sub_root));
    assert(this->cmp(*sub_root, *sub_root->_zt_right));
    assert(sub_root->_zt_right->_zt_parent == sub_root);

    this->dbg_verify_consistency(sub_root->_zt_right, sub_root, upper_bound);
  }

  if (sub_root->_zt_left != nullptr) {
    assert(RankGetter::get_rank(*sub_root->_zt_left) <=
           RankGetter::get_rank(*sub_root));
    assert(!this->cmp(*sub_root, *sub_root->_zt_left));
    assert(sub_root->_zt_left->_zt_parent == sub_root);

    this->dbg_verify_consistency(sub_root->_zt_left, lower_bound, sub_root);
  }
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::dump_to_dot(
    const std::string & filename) const
{
  this->dump_to_dot_base(
      filename, [&](const Node * node) { return NodeTraits::get_id(node); });
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
template <class NodeNameGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::dump_to_dot_base(
    const std::string & filename, NodeNameGetter name_getter) const
{
  std::ofstream dotfile;
  dotfile.open(filename);

  dotfile << "digraph G {\n";
  if (this->root != nullptr) {
    this->output_node_base(this->root, dotfile, name_getter);
  }
  dotfile << "}\n";
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
template <class NodeNameGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::output_node_base(
    const Node * node, std::ofstream & out, NodeNameGetter name_getter) const
{
  if (node == nullptr) {
    return;
  }

  out << "  " << std::to_string((long unsigned int)node) << "[ label=\""
      << name_getter(node) << "\"]\n";

  if (node->NB::_zt_parent != nullptr) {
    std::string label;
    if (node->NB::_zt_parent->NB::_zt_left == node) {
      label = std::string("L");
    } else {
      label = std::string("R");
    }

    out << "  " << std::to_string((long unsigned int)node->NB::_zt_parent)
        << " -> " << std::to_string((long unsigned int)node) << "[ label=\""
        << label << "\"]\n";
  }

  this->output_node_base(node->NB::_zt_left, out, name_getter);
  this->output_node_base(node->NB::_zt_right, out, name_getter);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
template <class Comparable>
typename ZTree<Node, NodeTraits, Options, Tag, Compare,
               RankGetter>::template iterator<false>
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::find(
    const Comparable & query)
{
  Node * cur = this->root;
  Node * last_left = nullptr;

  while (cur != nullptr) {
    if (this->cmp(*cur, query)) {
      cur = cur->NB::_zt_right;
    } else {
      last_left = cur;
      cur = cur->NB::_zt_left;
    }
  }

  if ((last_left != nullptr) && (!this->cmp(query, *last_left))) {
    return iterator<false>(last_left);
  } else {
    return this->end();
  }
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
template <class Comparable>
typename ZTree<Node, NodeTraits, Options, Tag, Compare,
               RankGetter>::template const_iterator<false>
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::find(
    const Comparable & query) const
{
  return const_iterator<false>(const_cast<decltype(this)>(this)->find(query));
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
Node *
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::get_smallest() const
{
  Node * smallest = this->root;
  if (smallest == nullptr) {
    return nullptr;
  }

  while (smallest->NB::_zt_left != nullptr) {
    smallest = smallest->NB::_zt_left;
  }

  return smallest;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
Node *
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::get_largest() const
{
  Node * largest = this->root;
  if (largest == nullptr) {
    return nullptr;
  }

  while (largest->NB::_zt_right != nullptr) {
    largest = largest->NB::_zt_right;
  }

  return largest;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
typename ZTree<Node, NodeTraits, Options, Tag, Compare,
               RankGetter>::template const_iterator<false>
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::iterator_to(
    const Node & node) const
{
  return const_iterator<false>(&node);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
typename ZTree<Node, NodeTraits, Options, Tag, Compare,
               RankGetter>::template iterator<false>
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::iterator_to(
    Node & node)
{
  return iterator<false>(&node);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
typename ZTree<Node, NodeTraits, Options, Tag, Compare,
               RankGetter>::template const_iterator<false>
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::cbegin() const
{
  Node * smallest = this->get_smallest();
  if (smallest == nullptr) {
    return const_iterator<false>(nullptr);
  }

  return const_iterator<false>(smallest);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
typename ZTree<Node, NodeTraits, Options, Tag, Compare,
               RankGetter>::template const_iterator<false>
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::cend() const
{
  return const_iterator<false>(nullptr);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
typename ZTree<Node, NodeTraits, Options, Tag, Compare,
               RankGetter>::template const_iterator<false>
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::begin() const
{
  return this->cbegin();
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
typename ZTree<Node, NodeTraits, Options, Tag, Compare,
               RankGetter>::template iterator<false>
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::begin()
{
  Node * smallest = this->get_smallest();
  if (smallest == nullptr) {
    return iterator<false>(nullptr);
  }

  return iterator<false>(smallest);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
typename ZTree<Node, NodeTraits, Options, Tag, Compare,
               RankGetter>::template const_iterator<false>
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::end() const
{
  return this->cend();
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
typename ZTree<Node, NodeTraits, Options, Tag, Compare,
               RankGetter>::template iterator<false>
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::end()
{
  return iterator<false>(nullptr);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
typename ZTree<Node, NodeTraits, Options, Tag, Compare,
               RankGetter>::template const_iterator<true>
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::crbegin() const
{
  Node * largest = this->get_largest();
  if (largest == nullptr) {
    return const_iterator<true>(nullptr);
  }

  return const_iterator<true>(largest);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
typename ZTree<Node, NodeTraits, Options, Tag, Compare,
               RankGetter>::template const_iterator<true>
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::crend() const
{
  return const_iterator<true>(nullptr);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
typename ZTree<Node, NodeTraits, Options, Tag, Compare,
               RankGetter>::template const_iterator<true>
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::rbegin() const
{
  return this->crbegin();
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
typename ZTree<Node, NodeTraits, Options, Tag, Compare,
               RankGetter>::template const_iterator<true>
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::rend() const
{
  return this->crend();
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
typename ZTree<Node, NodeTraits, Options, Tag, Compare,
               RankGetter>::template iterator<true>
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::rbegin()
{
  Node * largest = this->get_largest();
  if (largest == nullptr) {
    return iterator<true>(nullptr);
  }

  return iterator<true>(largest);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
typename ZTree<Node, NodeTraits, Options, Tag, Compare,
               RankGetter>::template iterator<true>
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::rend()
{
  return iterator<true>(nullptr);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
size_t
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::size() const
{
  return this->s.get();
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
bool
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::empty() const
{
  return (this->root == nullptr);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::clear()
{
  this->root = nullptr;
  this->s.set(0);
}

} // namespace ygg

#endif // YGG_ZIPTREE_CPP
