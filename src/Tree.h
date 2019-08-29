//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panwave contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#pragma once

#include <vector>
#include <cassert>
#include <cmath>

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
    template<class Element, size_t k>
    class Tree {
    protected:
        size_t _height;
        size_t _leaf_count;
        std::vector<Element> _nodes;
        std::vector<bool> _mark;

    public:
        /**
         * Tree constructor.
         * @param height The height of the tree. A tree consisting only of a single
         * root node has height = 1.
         */
        Tree(size_t height) : _height(height) {
            assert(height != 0);

            this->_leaf_count = static_cast<size_t>(std::pow(k, height - 1));
            size_t total_nodes = (k * this->_leaf_count - 1) / (k - 1);

            this->_nodes.resize(total_nodes);
            this->_mark.resize(total_nodes);
        }

    protected:
        /**
         * Return the number of leaves in the tree.<br/>
         * Leaves are the nodes in the bottom (last) level of the tree which
         * themselves do not have children nodes.
         */
        size_t GetLeafCount() {
            return this->_leaf_count;
        }

        /**
         * Return the index of the first leaf node.
         */
        size_t GetFirstLeaf() {
            return this->_nodes.size() - this->_leaf_count;
        }

        /**
         * Return the index of the last leaf node.
         */
        size_t GetLastLeaf() {
            return this->_nodes.size() - 1;
        }

        /**
         * Returns true if node is a leaf node.
         * @param node The index of a node to check.
         */
        bool IsLeaf(size_t node) {
            return node >= this->GetFirstLeaf() && node <= this->GetLastLeaf();
        }

        /**
         * Get the child of a parent node.
         * @param parent The parent node index.
         * @param child_index The 0-based index of the child relative to parent.<br/>
         *                    The first child is 0.<br/>
         *                    A binary tree node would have child indices of [0,1].<br/>
         *                    A quad tree node would have child indicies [0,1,2,3].
         */
        size_t GetChild(size_t parent, size_t child_index) {
            assert(child_index < k);
            assert(parent < this->_nodes.size());

            return k * parent + 1 + child_index;
        }

        /**
         * Get the parent index for a child node.
         * @param child The child node index.
         */
        size_t GetParent(size_t child) {
            assert(child < this->_nodes.size());
            assert(child != 0);

            return (child - 1) / k;
        }

        /**
         * Return true if node is set as marked.
         * @param node The node index to check for mark state.
         */
        bool IsMarked(size_t node) {
            assert(node < this->_mark.size());

            return this->_mark[node];
        }

        /**
         * Set node as marked.
         * @param node The node index to mark.
         */
        void SetMark(size_t node) {
            assert(node < this->_mark.size());

            this->_mark[node] = true;
        }

        /**
         * Unmark all nodes in the tree.
         */
        void Unmark() {
            this->_mark.assign(this->_mark.size(), false);
        }
    };
}; // namespace panwave
