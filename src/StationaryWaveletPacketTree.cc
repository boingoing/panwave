//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panwave contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include <cassert>
#include <vector>

#include "StationaryWaveletPacketTree.h"
#include "Wavelet.h"
#include "WaveletMath.h"

using namespace panwave;

StationaryWaveletPacketTree::StationaryWaveletPacketTree(size_t height,
                                                         const Wavelet* wavelet,
                                                         PaddingMode padding_mode) :
    WaveletPacketTreeTemplateBase(height, wavelet),
    _padding_mode(padding_mode) {
}

void StationaryWaveletPacketTree::Decompose() {
    this->DecomposeNode(0);
}

void StationaryWaveletPacketTree::DecomposeNode(size_t node) {
    assert(node < this->_nodes.size());

    if (this->IsLeaf(node)) {
        return;
    }

    size_t nw = this->GetChild(node, 0);
    size_t ne = this->GetChild(node, 1);
    size_t sw = this->GetChild(node, 2);
    size_t se = this->GetChild(node, 3);

    WaveletMath::Decompose(&this->_nodes[node].signal,
                           this->_wavelet,
                           &this->_nodes[nw].signal,
                           &this->_nodes[sw].signal,
                           DyadicMode::Even,
                           this->_padding_mode);

    WaveletMath::Decompose(&this->_nodes[node].signal,
                           this->_wavelet,
                           &this->_nodes[ne].signal,
                           &this->_nodes[se].signal,
                           DyadicMode::Odd,
                           this->_padding_mode);

    this->DecomposeNode(nw);
    this->DecomposeNode(ne);
    this->DecomposeNode(sw);
    this->DecomposeNode(se);
}

void StationaryWaveletPacketTree::ReconstructNode(size_t node) {
    assert(node < this->_nodes.size());

    if (this->IsLeaf(node)) {
        return;
    }

    size_t nw = this->GetChild(node, 0);
    size_t ne = this->GetChild(node, 1);
    size_t sw = this->GetChild(node, 2);
    size_t se = this->GetChild(node, 3);

    ReconstructNode(nw);
    ReconstructNode(ne);
    ReconstructNode(sw);
    ReconstructNode(se);

    DyadicMode dyad_mode = DyadicMode::Even;
    const std::vector<double>* child_signal = nullptr;
    const std::vector<double>* reconstruction_filter = nullptr;

    if (this->IsMarked(nw)) {
        assert(!this->IsMarked(sw) && !this->IsMarked(ne) && !this->IsMarked(se));
        dyad_mode = DyadicMode::Even;
        child_signal = &this->_nodes[nw].signal;
        reconstruction_filter = &this->_wavelet->LowpassReconstructionFilter;
        this->SetMark(node);
    } else if (this->IsMarked(sw)) {
        assert(!this->IsMarked(nw) && !this->IsMarked(ne) && !this->IsMarked(se));
        dyad_mode = DyadicMode::Even;
        child_signal = &this->_nodes[sw].signal;
        reconstruction_filter = &this->_wavelet->HighpassReconstructionFilter;
        this->SetMark(node);
    } else if (this->IsMarked(ne)) {
        assert(!this->IsMarked(sw) && !this->IsMarked(nw) && !this->IsMarked(se));
        dyad_mode = DyadicMode::Odd;
        child_signal = &this->_nodes[ne].signal;
        reconstruction_filter = &this->_wavelet->LowpassReconstructionFilter;
        this->SetMark(node);
    } else if (this->IsMarked(se)) {
        assert(!this->IsMarked(sw) && !this->IsMarked(ne) && !this->IsMarked(nw));
        dyad_mode = DyadicMode::Odd;
        child_signal = &this->_nodes[se].signal;
        reconstruction_filter = &this->_wavelet->HighpassReconstructionFilter;
        this->SetMark(node);
    }

    if (child_signal != nullptr) {
        WaveletMath::Reconstruct(child_signal,
                                 reconstruction_filter,
                                 &this->_nodes[node].signal,
                                 this->_wavelet,
                                 this->_nodes[node].signal.size(),
                                 dyad_mode,
                                 this->_padding_mode);
    }
}

void StationaryWaveletPacketTree::ReconstructAccumulate(size_t leaf_node, std::vector<double>* accumulated_signal) {
    assert(this->IsLeaf(leaf_node));
    assert(accumulated_signal);

    this->Unmark();
    this->SetMark(leaf_node);

    this->ReconstructNode(0);
    const std::vector<double>* root_signal = this->GetRootSignal();

    assert(root_signal->size() == accumulated_signal->size());

    for (size_t i = 0; i < root_signal->size(); i++) {
        accumulated_signal->operator[](i) += root_signal->operator[](i);
    }
}

void StationaryWaveletPacketTree::Reconstruct(size_t level) {
    // If height is 1, we only have the root node so there's nothing to reconstruct.
    if (this->_height == 1) {
        return;
    }

    size_t leaf_count = this->GetLeafCount();
    size_t level_count = this->GetWaveletLevelCount();
    size_t first_leaf_index = this->GetFirstLeaf();

    assert(level < level_count);

    std::vector<double> reconstructed_signal;
    reconstructed_signal.resize(this->_nodes[0].signal.size());

    // First calculate the starting leaf index for the level.
    size_t starting_leaf = 0;
    size_t current_leaf_count = 4;
    size_t current_level_count = 2;
    while (current_leaf_count <= leaf_count) {
        size_t remainder = level % current_level_count;

        if (remainder >= current_level_count / 2) {
            starting_leaf += current_leaf_count / 2;
        }

        current_leaf_count *= 4;
        current_level_count *= 2;
    }

    // Now use the starting leaf index to calculate the remaining leaves in the row.
    for (size_t i = 0; i < level_count / 2; i++) {
        size_t current_leaf = starting_leaf;
        size_t current_multiplier = 4;
        size_t level_index = i;

        while (level_index != 0) {
            // Least significant bit is set.
            if (level_index & 0x1) {
                current_leaf += current_multiplier;
            }

            // Shift to next most significant bit.
            level_index = level_index >> 1;
            // Increase multiplier to next power of 4;
            current_multiplier *= 4;
        }

        // We only calculated half of the leaf indices (the even half)
        // but the odd halves immediately follow each even half so we
        // need to reconstruct current_leaf and current_leaf + 1.
        this->ReconstructAccumulate(first_leaf_index + current_leaf, &reconstructed_signal);
        this->ReconstructAccumulate(first_leaf_index + current_leaf + 1, &reconstructed_signal);
    }

    for (size_t i = 0; i < reconstructed_signal.size(); i++) {
        reconstructed_signal[i] /= level_count;
    }

    this->SetRootSignal(&reconstructed_signal);
}
