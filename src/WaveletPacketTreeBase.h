//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panwave contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#pragma once

#include <vector>
#include <cassert>

#include "Tree.h"

namespace panwave {
    /**
     * Base class for all wavelet packet tree specialization types.<br/>
     * This abstract class is an interface to hold methods common to
     * various wavelet packet trees.
     */
    class WaveletPacketTreeBase {
    public:
        /**
         * Perform a wavelet packet tree decomposition.<br/>
         * Starting with the root node, decomposes recursively every node in
         * the tree stopping at the leaf nodes.<br/>
         * This is not a sparse decomposition, all nodes will have decomposed
         * signal data after executing.
         * @see SetRootSignal
         */
        virtual void Decompose() = 0;

        /**
         * Reconstruct an isolated wavelet level.<br/>
         * Beginning at the leaf nodes, recursively reconstruct up all levels
         * of the tree stopping at the root node.<br/>
         * Upon completion, the root signal will contain the reconstructed
         * signal.
         * @param level The wavelet level we should isolate and reconstruct.
         * @see GetRootSignal
         */
        virtual void Reconstruct(size_t level) = 0;

        /**
         * Set the root node signal.<br/>
         * This signal data is used during decomposition to construct all
         * signal values in the tree.
         * @param signal Values from signal are copied into the root node.
         * @see Decompose
         */
        virtual void SetRootSignal(const std::vector<double>* signal) = 0;

        /**
         * Get a read-only view of the root node signal data.
         * @see Reconstruct
         */
        virtual const std::vector<double>* GetRootSignal() const = 0;

        /**
         * Get the number of wavelet levels this tree is capable of
         * isolating and reconstructing.
         */
        virtual size_t GetWaveletLevelCount() const = 0;
    };

    /**
     * This struct is just a container used to hold the signal data for each
     * node in the wavelet packet tree.
     */
    struct WaveletPacketTreeNodeData {
        std::vector<double> signal;
    };

    /**
     * A templated base class from which specialized wavelet packet tree
     * implementations can derive.<br/>
     * Template argument k is the number of children per node.
     */
    template<size_t k>
    class WaveletPacketTreeTemplateBase : public Tree<WaveletPacketTreeNodeData, k>, public WaveletPacketTreeBase {
    protected:
        const Wavelet* _wavelet;

    public:
        WaveletPacketTreeTemplateBase(size_t height, const Wavelet* wavelet) :
            Tree<WaveletPacketTreeNodeData, k>(height),
            _wavelet(wavelet) {
        }

        void SetRootSignal(const std::vector<double>* signal) {
            assert(signal);

            this->_nodes[0].signal.assign(signal->cbegin(), signal->cend());
        }

        const std::vector<double>* GetRootSignal() const {
            return &this->_nodes[0].signal;
        }

        size_t GetWaveletLevelCount() const {
            return static_cast<size_t>(std::pow(2, this->_height - 1));
        }
    };
}; // namespace panwave
