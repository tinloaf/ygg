#include "ziptree.hpp"

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

  // TODO this should be handled by the code below
  if (this->root == nullptr) {
    this->root = &node;
    return;
  }

  // First, search for insertion position.
  auto node_rank = RankGetter::get_rank(node);

  Node * current = this->root;
  Node * last = nullptr;

  while ((current != nullptr) &&
         (node_rank <= RankGetter::get_rank(*current))) {
    last = current;
    // On equality, go left
    if (this->cmp(*current, node)) {
      current = current->_zt_right;
    } else {
      current = current->_zt_left;
    }
  }

  // Place new node in place of old
  if (last != nullptr) {
    node._zt_parent = last;
    if (last->_zt_left == current) {
      last->_zt_left = &node;
    } else {
      assert(last->_zt_right == current);
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
Node *
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::zip(
    Node & old_root) noexcept
{
  Node * left_head = old_root._zt_left;
  Node * right_head = old_root._zt_right;

  Node * new_root = nullptr;
  Node * cur = old_root._pt_parent;

  bool last_from_left;

  // First one is special.
  // TODO simplify this code!
  if ((left_head != nullptr) && (right_head != nullptr)) {
    // Both are there, use the one with the smaller rank

    if (RankGetter::get_rank(*left_head) < RankGetter::get_rank(*right_head)) {
      last_from_left = true;

      if (cur == nullptr) {
	this->root = left_head;
	left_head->_zt_parent = nullptr;
      } else {
	if (cur->_zt_left == &old_root) {
	  cur->_zt_left = left_head;
	} else {
	  assert(cur->_zt_rigt == &old_root);
	  cur->_zt_right = left_head;
	}
	left_head->_zt_parent = cur;
      }
      cur = left_head;
      left_head = (left_head->_zt_right != nullptr) ? left_head->_zt_right
                                                    : left_head->_zt_left;
    } else {
      last_from_left = false;

      if (cur == nullptr) {
	this->root = right_head;
	right_head->_zt_parent = nullptr;
      } else {
	if (cur->_zt_left == &old_root) {
	  cur->_zt_left = right_head;
	} else {
	  assert(cur->_zt_right == right_head);
	  cur->_zt_right = right_head;
	}

	right_head->_zt_parent = cur;
      }
      cur = right_head;
      right_head = (right_head->_zt_left != nullptr) ? right_head->_zt_left
                                                     : right_head->_zt_right;
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
	assert(cur->_zt_rigt == &old_root);
	cur->_zt_right = left_head;
      }
      left_head->_zt_parent = cur;
    }
    cur = left_head;
    left_head = (left_head->_zt_right != nullptr) ? left_head->_zt_right
                                                  : left_head->_zt_left;
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
	assert(cur->_zt_right == right_head);
	cur->_zt_right = right_head;
      }

      right_head->_zt_parent = cur;
    }

    cur = right_head;
    right_head = (right_head->_zt_left != nullptr) ? right_head->_zt_left
                                                   : right_head->_zt_right;
  }

  // Now, walk down left and right
  while ((left_head != nullptr) || (right_head != nullptr)) {
    if ((right_head == nullptr) || (RankGetter::get_rank(*left_head) <
                                    RankGetter::get_rank(*right_head))) {
      // Use left

      if (new_root == nullptr) { // TODO we don't need to check this every time
	new_root = left_head;
      }

      if (last_from_left) {
	// just pass on
      } else {
	// last was from the right, we can just use its left child.
	cur->_zt_left = left_head;
	left_head->_zt_parent = cur;
      }

      cur = left_head;
      left_head = (left_head->_zt_right != nullptr) ? left_head->_zt_right
                                                    : left_head->_zt_left;
      last_from_left = true;
    } else {
      // use right
      if (new_root == nullptr) { // TODO we don't need to check this every time
	new_root = right_head;
      }

      if (!last_from_left) {
	// just pass on
      } else {
	// last was from the left, we can just use its right child.
	cur->_zt_right = right_head;
	right_head->_zt_parent = cur;
      }
      cur = right_head;
      right_head = (right_head->_zt_left != nullptr) ? right_head->_zt_left
                                                     : right_head->_zt_right;
      last_from_left = false;
    }
  }

  return new_root;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare, RankGetter>::dbg_verify() const
{
  this->dbg_verify_consistency(this->root);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare,
          class RankGetter>
void
ZTree<Node, NodeTraits, Options, Tag, Compare,
      RankGetter>::dbg_verify_consistency(Node * sub_root) const
{
  if (sub_root == nullptr) {
    return;
  }

  if (sub_root->_zt_parent == nullptr) {
    assert(this->root == sub_root);
  } else {
    assert(this->root != sub_root);
  }

  if (sub_root->_zt_right != nullptr) {
    assert(RankGetter::get_rank(*sub_root->_zt_right) <
           RankGetter::get_rank(*sub_root));
    assert(this->cmp(*sub_root, *sub_root->_zt_right));
    assert(sub_root->_zt_right->_zt_parent == sub_root);

    this->dbg_verify_consistency(sub_root->_zt_right);
  }

  if (sub_root->_zt_left != nullptr) {
    assert(RankGetter::get_rank(*sub_root->_zt_left) <=
           RankGetter::get_rank(*sub_root));
    assert(!this->cmp(*sub_root, *sub_root->_zt_left));
    assert(sub_root->_zt_left->_zt_parent == sub_root);

    this->dbg_verify_consistency(sub_root->_zt_left);
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
  this->output_node_base(this->root, dotfile, name_getter);
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

} // namespace ygg
