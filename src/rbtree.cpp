#ifndef YGG_RBTREE_CPP
#define YGG_RBTREE_CPP

#include "rbtree.hpp"

namespace ygg {

namespace rbtree_internal {

template <class Node>
void
ColorParentStorage<Node, true>::set_color(Color new_color)
{
  if (new_color == Color::RED) {
    this->parent = (Node *)((size_t)this->parent | 1);
  } else {
    this->parent = (Node *)((size_t)this->parent & ~((size_t)1));
  }
}

template <class Node>
ygg::rbtree_internal::Color
ColorParentStorage<Node, true>::get_color() const
{
  if ((size_t)this->parent & 1) {
    return Color::RED;
  } else {
    return Color::BLACK;
  }
}

template <class Node>
void
ColorParentStorage<Node, true>::set_parent(Node * new_parent)
{
  this->parent = (Node *)((size_t)new_parent | ((size_t)this->parent & 1));
}

template <class Node>
Node *
ColorParentStorage<Node, true>::get_parent() const
{
  return (Node *)((size_t)this->parent & (~((size_t)1)));
}

template <class Node>
void
ColorParentStorage<Node, true>::swap_color_with(
    ColorParentStorage<Node, true> & other)
{
  // TODO make this more efficient?
  Color tmp = other.get_color();
  other.set_color(this->get_color());
  this->set_color(tmp);
}

template <class Node>
void
ColorParentStorage<Node, true>::swap_parent_with(
    ColorParentStorage<Node, true> & other)
{
  // TODO make this more efficient?
  Node * tmp = other.get_parent();
  other.set_parent(this->get_parent());
  this->set_parent(tmp);
}

// TODO have a 'swap both' operator!

template <class Node>
void
ColorParentStorage<Node, false>::set_color(Color new_color)
{
  this->color = new_color;
}

template <class Node>
ygg::rbtree_internal::Color
ColorParentStorage<Node, false>::get_color() const
{
  return this->color;
}

template <class Node>
void
ColorParentStorage<Node, false>::set_parent(Node * new_parent)
{
  this->parent = new_parent;
}

template <class Node>
Node *
ColorParentStorage<Node, false>::get_parent() const
{
  return this->parent;
}

template <class Node>
void
ColorParentStorage<Node, false>::swap_color_with(
    ColorParentStorage<Node, false> & other)
{
  std::swap(this->color, other.color);
}

template <class Node>
void
ColorParentStorage<Node, false>::swap_parent_with(
    ColorParentStorage<Node, false> & other)
{
  std::swap(this->parent, other.parent);
}

template <class Node, class Tag, bool compress_color>
size_t
RBTreeNodeBaseImpl<Node, Tag, compress_color>::get_depth() const
    noexcept
{
  size_t depth = 0;
  Node * n = (Node *)this;

  while (n->get_parent() != nullptr) {
    depth++;
    n = n->get_parent();
  }

  return depth;
}

template <class Node, class Tag, bool compress_color>
void
RBTreeNodeBaseImpl<Node, Tag, compress_color>::set_color(Color new_color)
{
  this->_color_and_parent.set_color(new_color);
}

template <class Node, class Tag, bool compress_color>
Color
RBTreeNodeBaseImpl<Node, Tag, compress_color>::get_color() const
{
  return this->_color_and_parent.get_color();
}

template <class Node, class Tag, bool compress_color>
Node *
RBTreeNodeBaseImpl<Node, Tag, compress_color>::get_parent() const
{
  return this->_color_and_parent.get_parent();
}

  template <class Node, class Tag, bool compress_color>
Node *
RBTreeNodeBaseImpl<Node, Tag, compress_color>::get_left() const
{
  return this->_rbt_left;
}

  template <class Node, class Tag, bool compress_color>
Node *
RBTreeNodeBaseImpl<Node, Tag, compress_color>::get_right() const
{
  return this->_rbt_right;
}

  
template <class Node, class Tag, bool compress_color>
void
RBTreeNodeBaseImpl<Node, Tag, compress_color>::set_parent(Node * new_parent)
{
  this->_color_and_parent.set_parent(new_parent);
}

template <class Node, class Tag, bool compress_color>
void
RBTreeNodeBaseImpl<Node, Tag, compress_color>::swap_color_with(Node * other)
{
  this->_color_and_parent.swap_color_with(other->_color_and_parent);
}

template <class Node, class Tag, bool compress_color>
void
RBTreeNodeBaseImpl<Node, Tag, compress_color>::swap_parent_with(Node * other)
{
  this->_color_and_parent.swap_parent_with(other->_color_and_parent);
}

} // namespace rbtree_internal

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
RBTree<Node, NodeTraits, Options, Tag, Compare>::RBTree() : root(nullptr)
{}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <bool on_equality_prefer_left>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::insert_leaf_base(Node & node,
                                                                  Node * start)
{
  node.NB::_rbt_right = nullptr;
  node.NB::_rbt_left = nullptr;

  Node * parent = start;
  Node * cur = start;

  while (cur != nullptr) {
    parent = cur;

    // TODO constexpr - if
    if (on_equality_prefer_left) {
      if (this->cmp(*cur, node)) {
	cur = cur->NB::_rbt_right;
      } else {
	cur = cur->NB::_rbt_left;
      }
    } else {
      if (this->cmp(node, *cur)) {
	cur = cur->NB::_rbt_left;
      } else {
	cur = cur->NB::_rbt_right;
      }
    }
  }

  if (parent == nullptr) {
    // new root!
    node.NB::set_parent(nullptr);
    node.NB::set_color(rbtree_internal::Color::BLACK);
    this->root = &node;
    NodeTraits::leaf_inserted(node);
  } else {
    node.NB::set_parent(parent);
    node.NB::set_color(rbtree_internal::Color::RED);

    if (this->cmp(node, *parent)) {
      parent->NB::_rbt_left = &node;
    } else if (this->cmp(*parent, node)) {
      parent->NB::_rbt_right = &node;
    } else {
      // assert(multiple);

      // TODO constexpr - if
      if (!Options::multiple) {
	return;
      }

      // TODO constexpr - if
      if (on_equality_prefer_left) {
	parent->NB::_rbt_left = &node;
      } else {
	parent->NB::_rbt_right = &node;
      }
    }

    NodeTraits::leaf_inserted(node);
    this->fixup_after_insert(&node);
  }

  return;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::rotate_left(Node * parent)
{
  Node * right_child = parent->NB::_rbt_right;
  parent->NB::_rbt_right = right_child->NB::_rbt_left;
  if (right_child->NB::_rbt_left != nullptr) {
    right_child->NB::_rbt_left->NB::set_parent(parent);
  }

  Node * parents_parent = parent->NB::get_parent();

  right_child->NB::_rbt_left = parent;
  right_child->NB::set_parent(parents_parent);

  if (parents_parent != nullptr) {
    if (parents_parent->NB::_rbt_left == parent) {
      parents_parent->NB::_rbt_left = right_child;
    } else {
      parents_parent->NB::_rbt_right = right_child;
    }
  } else {
    this->root = right_child;
  }

  parent->NB::set_parent(right_child);

  NodeTraits::rotated_left(*parent);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::rotate_right(Node * parent)
{
  Node * left_child = parent->NB::_rbt_left;
  parent->NB::_rbt_left = left_child->NB::_rbt_right;
  if (left_child->NB::_rbt_right != nullptr) {
    left_child->NB::_rbt_right->NB::set_parent(parent);
  }

  Node * parents_parent = parent->NB::get_parent();

  left_child->NB::_rbt_right = parent;
  left_child->NB::set_parent(parents_parent);

  if (parents_parent != nullptr) {
    if (parents_parent->NB::_rbt_left == parent) {
      parents_parent->NB::_rbt_left = left_child;
    } else {
      parents_parent->NB::_rbt_right = left_child;
    }
  } else {
    this->root = left_child;
  }

  parent->NB::set_parent(left_child);

  NodeTraits::rotated_right(*parent);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::fixup_after_insert(Node * node)
{
  // Does not happen: We only call this if we are not the root.
  /*
  if (node->NB::get_parent() == nullptr) {
    node->NB::set_color(rbtree_internal::Color::BLACK);
    return;
  }
  */

  while (
      (node->NB::get_parent()->NB::get_color() ==
       rbtree_internal::Color::RED) &&
      (this->get_uncle(node) != nullptr) &&
      (this->get_uncle(node)->NB::get_color() == rbtree_internal::Color::RED)) {
    Node * parent = node->NB::get_parent();
    parent->NB::set_color(rbtree_internal::Color::BLACK);
    this->get_uncle(node)->NB::set_color(rbtree_internal::Color::BLACK);

    Node * grandparent = parent->NB::get_parent();
    if (grandparent->NB::get_parent() !=
        nullptr) { // never iterate into the root
      grandparent->NB::set_color(rbtree_internal::Color::RED);
      node = grandparent;
    } else {
      // Don't recurse into the root; don't color it red. We could immediately
      // re-color it black.
      return;
    }
  }

  if (node->NB::get_parent()->NB::get_color() ==
      rbtree_internal::Color::BLACK) {
    return;
  }

  Node * parent = node->NB::get_parent();
  Node * grandparent = parent->NB::get_parent();

  if (grandparent->NB::_rbt_left == parent) {
    if (parent->NB::_rbt_right == node) {
      // 'folded in' situation
      this->rotate_left(parent);
      node->NB::set_color(rbtree_internal::Color::BLACK);
    } else {
      // 'straight' situation
      parent->NB::set_color(rbtree_internal::Color::BLACK);
    }

    this->rotate_right(grandparent);
  } else {
    if (parent->NB::_rbt_left == node) {
      // 'folded in'
      this->rotate_right(parent);
      node->NB::set_color(rbtree_internal::Color::BLACK);
    } else {
      // 'straight'
      parent->NB::set_color(rbtree_internal::Color::BLACK);
    }
    this->rotate_left(grandparent);
  }

  grandparent->NB::set_color(rbtree_internal::Color::RED);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::insert(Node & node)
{
  this->s.add(1);
  this->insert_leaf_base<true>(node, this->root);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::insert_left_leaning(
    Node & node)
{
  this->s.add(1);
  this->insert_leaf_base<true>(node, this->root);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::insert_right_leaning(
    Node & node)
{
  this->s.add(1);
  this->insert_leaf_base<false>(node, this->root);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::insert(Node & node,
                                                        Node & hint)
{
  // find parent
  Node * parent = &hint;

  /* We need to walk up if:
   *  - we're larger than the parent and in its left subtree
   *  - we're smaller than the parent and in its right subtree
   */
  while (
      (parent->NB::get_parent() != nullptr) &&
      (((parent->NB::get_parent()->NB::_rbt_left == parent) &&
        (this->cmp(*parent->NB::get_parent(),
                   node))) || // left subtree, parent should go before node
       ((parent->NB::get_parent()->NB::_rbt_right == parent) &&
        (this->cmp(node,
                   *parent->NB::get_parent()))))) { // right subtree, node
                                                    // should go before parent
    parent = parent->NB::get_parent();
  }

  if (parent->NB::_rbt_left != nullptr) {
    parent = parent->NB::_rbt_left;
    this->insert_leaf_base<false>(node, parent);
  } else {
    this->insert_leaf_base<true>(node, parent);
  }
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::insert(
    Node & node,
    RBTree<Node, NodeTraits, Options, Tag, Compare>::iterator<false> hint)
{
  if (hint == this->end()) {
    // special case: insert at the end
    Node * parent = this->root;

    if (parent == nullptr) {
      this->insert_leaf_base<false>(node, parent);
    } else {
      while (parent->NB::_rbt_right != nullptr) {
	parent = parent->NB::_rbt_right;
      }
      this->insert_leaf_base<false>(node, parent);
    }
  } else {
    this->insert(node, *hint);
  }
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::clear()
{
  this->root = nullptr;
  this->s.set(0);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
Node *
RBTree<Node, NodeTraits, Options, Tag, Compare>::get_uncle(Node * node) const
{
  Node * parent = node->NB::get_parent();
  Node * grandparent = parent->NB::get_parent();

  if (grandparent->NB::_rbt_left == parent) {
    return grandparent->NB::_rbt_right;
  } else {
    return grandparent->NB::_rbt_left;
  }
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
bool
RBTree<Node, NodeTraits, Options, Tag, Compare>::verify_black_root() const
{
  return ((this->root == nullptr) ||
          (this->root->NB::get_color() == rbtree_internal::Color::BLACK));
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
bool
RBTree<Node, NodeTraits, Options, Tag, Compare>::verify_black_paths(
    const Node * node, unsigned int * path_length) const
{
  unsigned int left_length, right_length;

  if (node->NB::_rbt_left == nullptr) {
    left_length = 0;
  } else {
    if (!this->verify_black_paths(node->NB::_rbt_left, &left_length)) {
      return false;
    }
  }

  if (node->NB::_rbt_right == nullptr) {
    right_length = 0;
  } else {
    if (!this->verify_black_paths(node->NB::_rbt_right, &right_length)) {
      return false;
    }
  }

  if (left_length != right_length) {
    return false;
  }

  if (node->NB::get_color() == rbtree_internal::Color::BLACK) {
    *path_length = left_length + 1;
  } else {
    *path_length = left_length;
  }

  return true;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
bool
RBTree<Node, NodeTraits, Options, Tag, Compare>::verify_red_black(
    const Node * node) const
{
  if (node == nullptr) {
    return true;
  }

  if (node->NB::get_color() == rbtree_internal::Color::RED) {
    if ((node->NB::_rbt_right != nullptr) &&
        (node->NB::_rbt_right->NB::get_color() ==
         rbtree_internal::Color::RED)) {
      return false;
    }

    if ((node->NB::_rbt_left != nullptr) &&
        (node->NB::_rbt_left->NB::get_color() == rbtree_internal::Color::RED)) {
      return false;
    }
  }

  return this->verify_red_black(node->NB::_rbt_left) &&
         this->verify_red_black(node->NB::_rbt_right);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
bool
RBTree<Node, NodeTraits, Options, Tag, Compare>::verify_order() const
{
  for (const Node & n : *this) {
    if (n.NB::_rbt_left != nullptr) {
      // left may not be larger
      if (this->cmp(n, *(n.NB::_rbt_left))) {
	assert(false);
	return false;
      }
    }

    if (n.NB::_rbt_right != nullptr) {
      // right may not be smaller
      if (this->cmp(*(n.NB::_rbt_right), n)) {
	assert(false);
	return false;
      }
    }
  }

  return true;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
bool
RBTree<Node, NodeTraits, Options, Tag, Compare>::verify_tree() const
{
  if (this->root == nullptr) {
    return true;
  }

  Node * cur = this->root;
  while (cur->NB::_rbt_left != nullptr) {
    cur = cur->NB::_rbt_left;
    if (cur->NB::_rbt_left == cur) {
      assert(false);
      return (false);
    }
  }

  std::set<Node *> seen;

  while (cur != nullptr) {
    if (seen.find(cur) != seen.end()) {
      assert(false);
      return false;
    }
    seen.insert(cur);

    if (cur->NB::_rbt_left != nullptr) {
      if (cur->NB::_rbt_left->NB::get_parent() != cur) {
	assert(false);
	return false;
      }
      if (cur->NB::_rbt_right == cur) {
	assert(false);
	return false;
      }
    }

    if (cur->NB::_rbt_right != nullptr) {
      if (cur->NB::_rbt_right->NB::get_parent() != cur) {
	assert(false);
	return false;
      }
      if (cur->NB::_rbt_right == cur) {
	assert(false);
	return false;
      }
    }

    /*
     * Begin: find the next-largest vertex
     */
    if (cur->NB::_rbt_right != nullptr) {
      // go to smallest larger-or-equal child
      cur = cur->NB::_rbt_right;
      while (cur->NB::_rbt_left != nullptr) {
	cur = cur->NB::_rbt_left;
      }
    } else {
      // go up

      // skip over the nodes already visited
      // TODO have a 'parents_left_child_is' / 'parents_right_child_is'
      // function?
      while ((cur->NB::get_parent() != nullptr) &&
             (cur->NB::get_parent()->NB::_rbt_right ==
              cur)) { // these are the nodes which are smaller and were already
	              // visited
	cur = cur->NB::get_parent();
      }

      // go one further up
      if (cur->NB::get_parent() == nullptr) {
	// done
	cur = nullptr;
      } else {
	// go up
	cur = cur->NB::get_parent();
      }
    }
    /*
     * End: find the next-largest vertex
     */
  }

  return true;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
bool
RBTree<Node, NodeTraits, Options, Tag, Compare>::verify_integrity() const
{
  unsigned int dummy;

  bool tree_okay = this->verify_tree();
  bool root_okay = this->verify_black_root();
  bool paths_okay =
      (this->root == nullptr) || this->verify_black_paths(this->root, &dummy);
  bool children_okay = this->verify_red_black(this->root);

  bool order_okay = this->verify_order();

  assert(root_okay && paths_okay && children_okay && tree_okay && order_okay);

  (void)dummy;

  return root_okay && paths_okay && children_okay && tree_okay && order_okay;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <class NodeNameGetter>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::dump_to_dot_base(
    const std::string & filename, NodeNameGetter name_getter) const
{
  std::ofstream dotfile;
  dotfile.open(filename);
  dotfile << "digraph G {\n";
  this->output_node_base(this->root, dotfile, name_getter);
  dotfile << "}\n";
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::dump_to_dot(
    const std::string & filename) const
{
  this->dump_to_dot_base(
      filename, [&](const Node * node) { return NodeTraits::get_id(node); });
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <class NodeNameGetter>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::output_node_base(
    const Node * node, std::ofstream & out, NodeNameGetter name_getter) const
{
  if (node == nullptr) {
    return;
  }

  std::string color;
  if (node->NB::get_color() == rbtree_internal::Color::BLACK) {
    color = "black";
  } else {
    color = "red";
  }

  out << "  " << std::to_string((long unsigned int)node) << "[ color=" << color
      << " label=\"" << name_getter(node) << "\"]\n";

  if (node->NB::get_parent() != nullptr) {
    std::string label;
    if (node->NB::get_parent()->NB::_rbt_left == node) {
      label = std::string("L");
    } else {
      label = std::string("R");
    }

    out << "  " << std::to_string((long unsigned int)node->NB::get_parent())
        << " -> " << std::to_string((long unsigned int)node) << "[ label=\""
        << label << "\"]\n";
  }

  this->output_node_base(node->NB::_rbt_left, out, name_getter);
  this->output_node_base(node->NB::_rbt_right, out, name_getter);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
size_t
RBTree<Node, NodeTraits, Options, Tag, Compare>::size() const
{
  return this->s.get();
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
bool
RBTree<Node, NodeTraits, Options, Tag, Compare>::empty() const
{
  return this->root == nullptr;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::swap_nodes(Node * n1,
                                                            Node * n2,
                                                            bool swap_colors)
{
  if (n1->NB::get_parent() == n2) {
    this->swap_neighbors(n2, n1);
  } else if (n2->NB::get_parent() == n1) {
    this->swap_neighbors(n1, n2);
  } else {
    this->swap_unrelated_nodes(n1, n2);
  }

  if (!swap_colors) {
    n1->swap_color_with(n2);
  }

  NodeTraits::swapped(*n1, *n2);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::replace_node(
    Node * to_be_replaced, Node * replace_with)
{
  Node * parent = to_be_replaced->get_parent();
  if (parent != nullptr) {
    if (parent->_rbt_left == to_be_replaced) {
      parent->_rbt_left = replace_with;
    } else {
      parent->_rbt_right = replace_with;
    }
  } else {
    this->root = replace_with;
  }
  replace_with->set_parent(parent);

  // TODO callback?
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::swap_neighbors(Node * parent,
                                                                Node * child)
{
  child->NB::set_parent(parent->NB::get_parent());
  parent->NB::set_parent(child);
  if (child->NB::get_parent() != nullptr) {
    if (child->NB::get_parent()->NB::_rbt_left == parent) {
      child->NB::get_parent()->NB::_rbt_left = child;
    } else {
      child->NB::get_parent()->NB::_rbt_right = child;
    }
  } else {
    this->root = child;
  }

  if (parent->NB::_rbt_left == child) {
    parent->NB::_rbt_left = child->NB::_rbt_left;
    if (parent->NB::_rbt_left != nullptr) {
      parent->NB::_rbt_left->NB::set_parent(parent);
    }
    child->NB::_rbt_left = parent;

    std::swap(parent->NB::_rbt_right, child->NB::_rbt_right);
    if (child->NB::_rbt_right != nullptr) {
      child->NB::_rbt_right->NB::set_parent(child);
    }
    if (parent->NB::_rbt_right != nullptr) {
      parent->NB::_rbt_right->NB::set_parent(parent);
    }
  } else {
    parent->NB::_rbt_right = child->NB::_rbt_right;
    if (parent->NB::_rbt_right != nullptr) {
      parent->NB::_rbt_right->NB::set_parent(parent);
    }
    child->NB::_rbt_right = parent;

    std::swap(parent->NB::_rbt_left, child->NB::_rbt_left);
    if (child->NB::_rbt_left != nullptr) {
      child->NB::_rbt_left->NB::set_parent(child);
    }
    if (parent->NB::_rbt_left != nullptr) {
      parent->NB::_rbt_left->NB::set_parent(parent);
    }
  }
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::swap_unrelated_nodes(Node * n1,
                                                                      Node * n2)
{
  std::swap(n1->NB::_rbt_left, n2->NB::_rbt_left);
  if (n1->NB::_rbt_left != nullptr) {
    n1->NB::_rbt_left->NB::set_parent(n1);
  }
  if (n2->NB::_rbt_left != nullptr) {
    n2->NB::_rbt_left->NB::set_parent(n2);
  }

  std::swap(n1->NB::_rbt_right, n2->NB::_rbt_right);
  if (n1->NB::_rbt_right != nullptr) {
    n1->NB::_rbt_right->NB::set_parent(n1);
  }
  if (n2->NB::_rbt_right != nullptr) {
    n2->NB::_rbt_right->NB::set_parent(n2);
  }

  n1->NB::swap_parent_with(n2);

  if (n1->NB::get_parent() != nullptr) {
    if (n1->NB::get_parent()->NB::_rbt_right == n2) {
      n1->NB::get_parent()->NB::_rbt_right = n1;
    } else {
      n1->NB::get_parent()->NB::_rbt_left = n1;
    }
  } else {
    this->root = n1;
  }
  if (n2->NB::get_parent() != nullptr) {
    if (n2->NB::get_parent()->NB::_rbt_right == n1) {
      n2->NB::get_parent()->NB::_rbt_right = n2;
    } else {
      n2->NB::get_parent()->NB::_rbt_left = n2;
    }
  } else {
    this->root = n2;
  }
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::remove_to_leaf(Node & node)
{
  Node * cur = &node;
  Node * child = &node;

  if ((cur->NB::_rbt_right != nullptr) && (cur->NB::_rbt_left != nullptr)) {
    // Find the minimum of the larger-or-equal children
    child = cur->NB::_rbt_right;
    while (child->NB::_rbt_left != nullptr) {
      child = child->NB::_rbt_left;
    }
  } else if (cur->NB::_rbt_left != nullptr) {
    // Only a left child. This must be red and cannot have further children
    // (otherwise, black-balance would be violated)
    child = child->NB::_rbt_left;
  }

  if (child != &node) {
    this->swap_nodes(&node, child, false);
  }
  // Now, node is a pseudo-leaf with the color of child.

  // Node cannot have a left child, so if it has a right child, it must be red,
  // thus node must be black
  if (node.NB::_rbt_right != nullptr) {
    // replace node with its child and color the child black.
    auto right_child = node.NB::_rbt_right;

    /*
     * If the delete_leaf() callback is implemented, we need to actually swap
     * the right child with the node to be deleted, s.t. we still have a node
     * that we can call the callback on. If it is not implemented, we can just
     * replace the node with its child, not caring to "swap back".
     */
    // TODO C++17 mark this if constexpr
    if (&NodeTraits::delete_leaf == &RBDefaultNodeTraits<Node>::delete_leaf) {
      // Not overridden
      this->replace_node(&node, right_child);
      right_child->NB::set_color(rbtree_internal::Color::BLACK);
    } else {
      // Overridden
      this->swap_nodes(&node, right_child, true);

      NodeTraits::delete_leaf(node);

      right_child->NB::set_color(rbtree_internal::Color::BLACK);
      right_child->NB::_rbt_right =
          nullptr; // this stored the node to be deleted…
      // TODO null the pointers in node?
    }

    NodeTraits::deleted_below(*right_child);

    return; // no fixup necessary
  }

  // Node has no children, so we have to just delete it, which is no problem if
  // we are red. Otherwise, we must start a fixup at the parent.
  bool deleted_left = false;
  NodeTraits::delete_leaf(node);
  if (node.NB::get_parent() != nullptr) {
    if (node.NB::get_parent()->NB::_rbt_left == &node) {
      node.NB::get_parent()->NB::_rbt_left = nullptr;
      deleted_left = true;
    } else {
      node.NB::get_parent()->NB::_rbt_right = nullptr;
    }

    NodeTraits::deleted_below(*node.NB::get_parent());
  } else {
    this->root = nullptr; // Tree is now empty!
    return;               // No fixup needed!
  }

  if (node.NB::get_color() == rbtree_internal::Color::BLACK) {
    this->fixup_after_delete(node.NB::get_parent(), deleted_left);
  }
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::fixup_after_delete(
    Node * parent, bool deleted_left)
{
  bool propagating_up = true;
  Node * sibling;

  while (propagating_up) {
    // We just deleted a black node from under parent.
    if (deleted_left) {
      sibling = parent->NB::_rbt_right;
    } else {
      sibling = parent->NB::_rbt_left;
    }

    // sibling must exist! If it didn't, then that branch would have had too few
    // blacks…
    if ((parent->NB::get_color() == rbtree_internal::Color::BLACK) &&
        (sibling->NB::get_color() == rbtree_internal::Color::BLACK) &&
        ((sibling->NB::_rbt_left == nullptr) ||
         (sibling->NB::_rbt_left->NB::get_color() ==
          rbtree_internal::Color::BLACK)) &&
        ((sibling->NB::_rbt_right == nullptr) ||
         (sibling->NB::_rbt_right->NB::get_color() ==
          rbtree_internal::Color::BLACK))) {

      // We can recolor and propagate up! (Case 3)
      sibling->NB::set_color(rbtree_internal::Color::RED);
      // Now everything below parent is okay, but the branch started in parent
      // lost a black!
      if (parent->NB::get_parent() == nullptr) {
	// Doesn't matter! parent is the root, no harm done.
	return;
      } else {
	// propagate up!
	deleted_left = parent->NB::get_parent()->NB::_rbt_left == parent;
	parent = parent->NB::get_parent();
      }
    } else { // could not recolor the sibling, do not propagate up
      propagating_up = false;
    }
  }

  if (sibling->NB::get_color() == rbtree_internal::Color::RED) {
    // Case 2
    sibling->NB::set_color(rbtree_internal::Color::BLACK);
    parent->NB::set_color(rbtree_internal::Color::RED);
    if (deleted_left) {
      this->rotate_left(parent);
      sibling = parent->NB::_rbt_right;
    } else {
      this->rotate_right(parent);
      sibling = parent->NB::_rbt_left;
    }
  }

  if ((sibling->NB::get_color() == rbtree_internal::Color::BLACK) &&
      ((sibling->NB::_rbt_left == nullptr) ||
       (sibling->NB::_rbt_left->NB::get_color() ==
        rbtree_internal::Color::BLACK)) &&
      ((sibling->NB::_rbt_right == nullptr) ||
       (sibling->NB::_rbt_right->NB::get_color() ==
        rbtree_internal::Color::BLACK))) {
    // case 4
    parent->NB::set_color(rbtree_internal::Color::BLACK);
    sibling->NB::set_color(rbtree_internal::Color::RED);

    return; // No further fixup necessary
  }

  if (deleted_left) {
    if ((sibling->NB::_rbt_right == nullptr) ||
        (sibling->NB::_rbt_right->NB::get_color() ==
         rbtree_internal::Color::BLACK)) {
      // left child of sibling must be red! This is the folded case. (Case 5)
      // Unfold!
      this->rotate_right(sibling);
      sibling->NB::set_color(rbtree_internal::Color::RED);
      // The new sibling is now the parent of the sibling
      sibling = sibling->NB::get_parent();
      sibling->NB::set_color(rbtree_internal::Color::BLACK);
    }

    // straight situation, case 6 applies!
    this->rotate_left(parent);

    parent->NB::swap_color_with(sibling);

    sibling->NB::_rbt_right->NB::set_color(rbtree_internal::Color::BLACK);
  } else {
    if ((sibling->NB::_rbt_left == nullptr) ||
        (sibling->NB::_rbt_left->NB::get_color() ==
         rbtree_internal::Color::BLACK)) {
      // right child of sibling must be red! This is the folded case. (Case 5)
      // Unfold!

      this->rotate_left(sibling);
      sibling->NB::set_color(rbtree_internal::Color::RED);
      // The new sibling is now the parent of the sibling
      sibling = sibling->NB::get_parent();
      sibling->NB::set_color(rbtree_internal::Color::BLACK);
    }

    // straight situation, case 6 applies!
    this->rotate_right(parent);
    parent->NB::swap_color_with(sibling);
    sibling->NB::_rbt_left->NB::set_color(rbtree_internal::Color::BLACK);
  }
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
void
RBTree<Node, NodeTraits, Options, Tag, Compare>::remove(Node & node)
{
  this->s.reduce(1);

  // TODO collapse this method
  this->remove_to_leaf(node);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
Node *
RBTree<Node, NodeTraits, Options, Tag, Compare>::get_smallest() const
{
  Node * smallest = this->root;
  if (smallest == nullptr) {
    return nullptr;
  }

  while (smallest->NB::_rbt_left != nullptr) {
    smallest = smallest->NB::_rbt_left;
  }

  return smallest;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
Node *
RBTree<Node, NodeTraits, Options, Tag, Compare>::get_largest() const
{
  Node * largest = this->root;
  if (largest == nullptr) {
    return nullptr;
  }

  while (largest->NB::_rbt_right != nullptr) {
    largest = largest->NB::_rbt_right;
  }

  return largest;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename RBTree<Node, NodeTraits, Options, Tag,
                Compare>::template const_iterator<false>
RBTree<Node, NodeTraits, Options, Tag, Compare>::iterator_to(
    const Node & node) const
{
  return const_iterator<false>(&node);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename RBTree<Node, NodeTraits, Options, Tag,
                Compare>::template iterator<false>
RBTree<Node, NodeTraits, Options, Tag, Compare>::iterator_to(Node & node)
{
  return iterator<false>(&node);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename RBTree<Node, NodeTraits, Options, Tag,
                Compare>::template const_iterator<false>
RBTree<Node, NodeTraits, Options, Tag, Compare>::cbegin() const
{
  Node * smallest = this->get_smallest();
  if (smallest == nullptr) {
    return const_iterator<false>(nullptr);
  }

  return const_iterator<false>(smallest);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename RBTree<Node, NodeTraits, Options, Tag,
                Compare>::template const_iterator<false>
RBTree<Node, NodeTraits, Options, Tag, Compare>::cend() const
{
  return const_iterator<false>(nullptr);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename RBTree<Node, NodeTraits, Options, Tag,
                Compare>::template const_iterator<false>
RBTree<Node, NodeTraits, Options, Tag, Compare>::begin() const
{
  return this->cbegin();
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename RBTree<Node, NodeTraits, Options, Tag,
                Compare>::template iterator<false>
RBTree<Node, NodeTraits, Options, Tag, Compare>::begin()
{
  Node * smallest = this->get_smallest();
  if (smallest == nullptr) {
    return iterator<false>(nullptr);
  }

  return iterator<false>(smallest);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename RBTree<Node, NodeTraits, Options, Tag,
                Compare>::template const_iterator<false>
RBTree<Node, NodeTraits, Options, Tag, Compare>::end() const
{
  return this->cend();
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename RBTree<Node, NodeTraits, Options, Tag,
                Compare>::template iterator<false>
RBTree<Node, NodeTraits, Options, Tag, Compare>::end()
{
  return iterator<false>(nullptr);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename RBTree<Node, NodeTraits, Options, Tag,
                Compare>::template const_iterator<true>
RBTree<Node, NodeTraits, Options, Tag, Compare>::crbegin() const
{
  Node * largest = this->get_largest();
  if (largest == nullptr) {
    return const_iterator<true>(nullptr);
  }

  return const_iterator<true>(largest);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename RBTree<Node, NodeTraits, Options, Tag,
                Compare>::template const_iterator<true>
RBTree<Node, NodeTraits, Options, Tag, Compare>::crend() const
{
  return const_iterator<true>(nullptr);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename RBTree<Node, NodeTraits, Options, Tag,
                Compare>::template const_iterator<true>
RBTree<Node, NodeTraits, Options, Tag, Compare>::rbegin() const
{
  return this->crbegin();
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename RBTree<Node, NodeTraits, Options, Tag,
                Compare>::template const_iterator<true>
RBTree<Node, NodeTraits, Options, Tag, Compare>::rend() const
{
  return this->crend();
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename RBTree<Node, NodeTraits, Options, Tag,
                Compare>::template iterator<true>
RBTree<Node, NodeTraits, Options, Tag, Compare>::rbegin()
{
  Node * largest = this->get_largest();
  if (largest == nullptr) {
    return iterator<true>(nullptr);
  }

  return iterator<true>(largest);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
typename RBTree<Node, NodeTraits, Options, Tag,
                Compare>::template iterator<true>
RBTree<Node, NodeTraits, Options, Tag, Compare>::rend()
{
  return iterator<true>(nullptr);
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <class Comparable, class Callbacks>
typename RBTree<Node, NodeTraits, Options, Tag,
                Compare>::template iterator<false>
RBTree<Node, NodeTraits, Options, Tag, Compare>::find(const Comparable & query,
                                                      Callbacks * cbs)
{
  Node * cur = this->root;
  cbs->init_root(cur);

  while (cur != nullptr) {
    if (this->cmp(*cur, query)) {
      cur = cur->NB::_rbt_right;
      cbs->descend_right(cur);
    } else if (this->cmp(query, *cur)) {
      cur = cur->NB::_rbt_left;
      cbs->descend_left(cur);
    } else {
      cbs->found(cur);
      return iterator<false>(cur);
    }
  }

  cbs->not_found();
  return this->end();
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <class Comparable>
typename RBTree<Node, NodeTraits, Options, Tag,
                Compare>::template iterator<false>
RBTree<Node, NodeTraits, Options, Tag, Compare>::find(const Comparable & query)
{
  Node * cur = this->root;
  Node * last_left = nullptr;

  while (cur != nullptr) {
    if (this->cmp(*cur, query)) {
      cur = cur->NB::_rbt_right;
    } else {
      last_left = cur;
      cur = cur->NB::_rbt_left;
    }
  }

  if ((last_left != nullptr) && (!this->cmp(query, *last_left))) {
    return iterator<false>(last_left);
  } else {
    return this->end();
  }
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <class Comparable>
typename RBTree<Node, NodeTraits, Options, Tag,
                Compare>::template const_iterator<false>
RBTree<Node, NodeTraits, Options, Tag, Compare>::find(
    const Comparable & query) const
{
  return const_iterator<false>(const_cast<decltype(this)>(this)->find(query));
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <class Comparable>
typename RBTree<Node, NodeTraits, Options, Tag,
                Compare>::template iterator<false>
RBTree<Node, NodeTraits, Options, Tag, Compare>::lower_bound(
    const Comparable & query)
{
  Node * cur = this->root;
  Node * last_left = nullptr;

  while (cur != nullptr) {
    if (this->cmp(*cur, query)) {
      cur = cur->NB::_rbt_right;
    } else {
      last_left = cur;
      cur = cur->NB::_rbt_left;
    }
  }

  if (last_left != nullptr) {
    return iterator<false>(last_left);
  } else {
    return this->end();
  }
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <class Comparable>
typename RBTree<Node, NodeTraits, Options, Tag,
                Compare>::template iterator<false>
RBTree<Node, NodeTraits, Options, Tag, Compare>::upper_bound(
    const Comparable & query)
{
  Node * cur = this->root;
  Node * last_left = nullptr;

  while (cur != nullptr) {
    if (this->cmp(query, *cur)) {
      last_left = cur;
      cur = cur->_rbt_left;
    } else {
      cur = cur->_rbt_right;
    }
  }

  if (last_left != nullptr) {
    return iterator<false>(last_left);
  } else {
    return this->end();
  }
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <class Comparable>
typename RBTree<Node, NodeTraits, Options, Tag,
                Compare>::template const_iterator<false>
RBTree<Node, NodeTraits, Options, Tag, Compare>::upper_bound(
    const Comparable & query) const
{
  return const_iterator<false>(
      const_cast<RBTree<Node, NodeTraits, Options, Tag, Compare> *>(this)
          ->upper_bound(query));
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
template <class Comparable>
typename RBTree<Node, NodeTraits, Options, Tag,
                Compare>::template const_iterator<false>
RBTree<Node, NodeTraits, Options, Tag, Compare>::lower_bound(
    const Comparable & query) const
{
  return const_iterator<false>(
      const_cast<RBTree<Node, NodeTraits, Options, Tag, Compare> *>(this)
          ->lower_bound(query));
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
Node *
RBTree<Node, NodeTraits, Options, Tag, Compare>::get_parent(Node * n)
{
  return n->NB::get_parent();
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
Node *
RBTree<Node, NodeTraits, Options, Tag, Compare>::get_left_child(Node * n)
{
  return n->NB::_rbt_left;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
Node *
RBTree<Node, NodeTraits, Options, Tag, Compare>::get_right_child(Node * n)
{
  return n->NB::_rbt_right;
}

template <class Node, class NodeTraits, class Options, class Tag, class Compare>
Node *
RBTree<Node, NodeTraits, Options, Tag, Compare>::get_root() const
{
  return this->root;
}

/*
template<class Node, class NodeTraits, class Options, class Tag, class Compare>
template<class Comparable>
typename RBTreeBaseBase<Node, NodeTraits, Compare>::template
const_iterator<false> RBTreeBaseBase<Node, NodeTraits,
Compare>::lower_bound(const Comparable & query) const
{
  Node * cur = this->root;
  while (cur != nullptr) {
    if (Compare()(query, *cur)) {
      cur = cur->NB::_rbt_left;
    } else if (Compare()(*cur, query)) {
      cur = cur->NB::_rbt_right;
    } else {
      cur = EqualityList::equality_list_find_first(cur);
      return const_iterator<false>(cur);
    }
  }

  return this->cend();
}
*/

} // namespace ygg

#endif // YGG_RBTREE_CPP
