//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panwave contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for
// full license information.
//-------------------------------------------------------------------------------------------------------

#ifndef TREE_H
#define TREE_H

#include <cassert>
#include <cmath>
#include <vector>

namespace panwave {

/**
 * A simple, generic k-ary tree implementation.<br/>
 * This class is designed to serve as the basis for the WaveletPacketTree
 * set of classes and, as such, doesn't provide much functionality to
 * use it as a stand-alone tree.<br/>
 * Template argument k defines the number of children each node in the tree
 * has.<br/>
 * If k=2 the tree is an ordinary binary tree.<br/>
 * If k=4 the tree is an ordinary quad tree.<br/>
 * Etc.<br/>
 * Nodes are physically allocated in a vector. The first node in the vector
 * is the root node. The next k nodes are the nodes in the second level of
 * the tree (these are the children of the root node) and the remaining
 * nodes in the vector continue this trend.
 */
template <class Element, size_t k>
class Tree {
 public:
  /**
   * Tree constructor.
   * @param height The height of the tree. A tree consisting only of a single
   * root node has height = 1.
   */
  explicit Tree(size_t height) : height_(height) {
    assert(height != 0);

    this->leaf_count_ = static_cast<size_t>(std::pow(k, height_ - 1));
    size_t const total_nodes = (k * this->leaf_count_ - 1) / (k - 1);

    this->nodes_.resize(total_nodes);
    this->mark_.resize(total_nodes);
  }

 protected:
  /**
   * Return the number of leaves in the tree.<br/>
   * Leaves are the nodes in the bottom (last) level of the tree which
   * themselves do not have children nodes.
   */
  size_t GetLeafCount() const { return this->leaf_count_; }

  /**
   * Return the index of the first leaf node.
   */
  size_t GetFirstLeaf() const {
    return this->nodes_.size() - this->leaf_count_;
  }

  /**
   * Return the index of the last leaf node.
   */
  size_t GetLastLeaf() const { return this->nodes_.size() - 1; }

  /**
   * Returns true if node is a leaf node.
   * @param node The index of a node to check.
   */
  bool IsLeaf(size_t node) const {
    return node >= this->GetFirstLeaf() && node <= this->GetLastLeaf();
  }

  /**
   * Get the child of a parent node.
   * @param parent The parent node index.
   * @param child_index The 0-based index of the child relative to parent.<br/>
   *                    The first child is 0.<br/>
   *                    A binary tree node would have child indices of
   * [0,1].<br/> A quad tree node would have child indicies [0,1,2,3].
   */
  size_t GetChild(size_t parent, size_t child_index) const {
    assert(child_index < k);
    assert(parent < this->nodes_.size());

    return k * parent + 1 + child_index;
  }

  /**
   * Get the parent index for a child node.
   * @param child The child node index.
   */
  size_t GetParent(size_t child) const {
    assert(child < this->nodes_.size());
    assert(child != 0);

    return (child - 1) / k;
  }

  Element& GetNodeData(size_t index) {
    assert(index < this->nodes_.size());

    return this->nodes_[index];
  }

  /**
   * Return true if node is set as marked.
   * @param node The node index to check for mark state.
   */
  bool IsMarked(size_t node) const {
    assert(node < this->mark_.size());

    return this->mark_[node];
  }

  /**
   * Set node as marked.
   * @param node The node index to mark.
   */
  void SetMark(size_t node) {
    assert(node < this->mark_.size());

    this->mark_[node] = true;
  }

  /**
   * Unmark all nodes in the tree.
   */
  void Unmark() { this->mark_.assign(this->mark_.size(), false); }

  /**
   * Get the height of the tree.
   *
   * @return size_t The height of the tree.
   * A height of one indicates only a root node with no children.
   */
  size_t GetHeight() const { return this->height_; }

 private:
  size_t height_;
  size_t leaf_count_ = 0;
  std::vector<Element> nodes_;
  std::vector<bool> mark_;
};

}  // namespace panwave

#endif  // TREE_H
