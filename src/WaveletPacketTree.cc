//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panwave contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include <cassert>
#include <cmath>

#include "WaveletPacketTree.h"
#include "WaveletPacketTreeBase.h"

using namespace panwave;

WaveletPacketTree::WaveletPacketTree(size_t height,
                                     const Wavelet* wavelet,
                                     DyadicMode dyadic_mode,
                                     PaddingMode padding_mode) :
    WaveletPacketTreeTemplateBase(height, wavelet),
    _dyadic_mode(dyadic_mode),
    _padding_mode(padding_mode) {
}

void WaveletPacketTree::Decompose() {
    this->DecomposeNode(0);
}

void WaveletPacketTree::Reconstruct(size_t level) {
    // Unmark all nodes in the tree.
    this->Unmark();

    // Mark only the leaf node for the level we want to isolate.
    this->IsolateLevel(level);

    // Reconstruct the signal at the requested level.
    this->ReconstructNode(0);
}

void WaveletPacketTree::IsolateLevel(size_t level) {
    assert(level < this->GetWaveletLevelCount());

    // This is a binary tree, the number of wavelet levels is equal to the
    // number of leaves.
    this->SetMark(this->GetFirstLeaf() + level);
}

void WaveletPacketTree::DecomposeNode(size_t node) {
    assert(node < this->_nodes.size());

    if (this->IsLeaf(node)) {
        return;
    }

    size_t left = this->GetChild(node, 0);
    size_t right = this->GetChild(node, 1);

    WaveletMath::Decompose(&this->_nodes[node].signal,
                           this->_wavelet,
                           &this->_nodes[left].signal,
                           &this->_nodes[right].signal,
                           this->_dyadic_mode,
                           this->_padding_mode);

    this->DecomposeNode(left);
    this->DecomposeNode(right);
}

void WaveletPacketTree::ReconstructNode(size_t node) {
    assert(node < this->_nodes.size());

    if (this->IsLeaf(node)) {
        return;
    }

    size_t left = this->GetChild(node, 0);
    size_t right = this->GetChild(node, 1);

    ReconstructNode(left);
    ReconstructNode(right);

    const std::vector<double>* child_signal = nullptr;
    const std::vector<double>* reconstruction_filter = nullptr;

    if (this->IsMarked(left)) {
        assert(!this->IsMarked(right));
        child_signal = &this->_nodes[left].signal;
        reconstruction_filter = &this->_wavelet->LowpassReconstructionFilter;
        this->SetMark(node);
    } else if (this->IsMarked(right)) {
        assert(!this->IsMarked(left));
        child_signal = &this->_nodes[right].signal;
        reconstruction_filter = &this->_wavelet->HighpassReconstructionFilter;
        this->SetMark(node);
    }

    if (child_signal != nullptr) {
        WaveletMath::Reconstruct(child_signal,
                                 reconstruction_filter,
                                 &this->_nodes[node].signal,
                                 this->_wavelet,
                                 this->_nodes[node].signal.size(),
                                 this->_dyadic_mode,
                                 this->_padding_mode);
    }
}
