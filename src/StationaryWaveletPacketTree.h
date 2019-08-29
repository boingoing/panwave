//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panwave contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#pragma once

#include <vector>

#include "Tree.h"
#include "Wavelet.h"
#include "WaveletMath.h"
#include "WaveletPacketTreeBase.h"

namespace panwave {
    /**
     * A wavelet packet tree which tries to keep the reconstructed signals
     * stationary in time. This has the effect of producing smoother
     * reconstructed signals compared to a conventional wavelet packet tree
     * decomposition.<br/>
     * This is implemented as a quad tree where the signal of each node is
     * decomposed into four children signals. The four signals produced are
     * the details and approximate coefficients downsampled dyadically in
     * both even and odd dyadic modes.
     * @see WaveletPacketTree
     */
    class StationaryWaveletPacketTree : public WaveletPacketTreeTemplateBase<4> {
    protected:
        PaddingMode _padding_mode;

    public:
        /**
         * Construct a StationaryWaveletPacketTree instance.<br/>
         * Root signal is initially unset. Set it before calling Decompose.
         * @param height Height of the tree. A tree with only one root node
         *               has height of 1.
         * @param wavelet Wavelet object used during decomposition /
         *                reconstruction.
         * @param padding_mode How we should pad the signal data during
         *                     decomposition / reconstruction. (default: Zeroes)
         * @see Wavelet
         * @see Decompose
         * @see Reconstruct
         */
        StationaryWaveletPacketTree(size_t height,
                                    const Wavelet* wavelet,
                                    PaddingMode padding_mode = PaddingMode::Zeroes);

        void Decompose();
        void Reconstruct(size_t level);

    protected:
        StationaryWaveletPacketTree(const StationaryWaveletPacketTree&);

        void DecomposeNode(size_t node);
        void ReconstructNode(size_t node);

        /**
         * Isolates one leaf and reconstructs the root signal. The root signal is accumulated
         * into accumulated_signal.
         * @param leaf_node The leaf node we want to isolate.
         * @param accumulated_signal Vector in which we will add the root signal after
         *                           isolating the leaf and reconstructing the signal.
         */
        void ReconstructAccumulate(size_t leaf_node, std::vector<double>* accumulated_signal);
    };
}; // namespace panwave
