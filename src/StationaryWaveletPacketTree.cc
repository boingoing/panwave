//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panwave contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for
// full license information.
//-------------------------------------------------------------------------------------------------------

#include "StationaryWaveletPacketTree.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <vector>

#include "Wavelet.h"
#include "WaveletMath.h"

namespace {

constexpr size_t ChildIndexNorthWest = 0;
constexpr size_t ChildIndexNorthEast = 1;
constexpr size_t ChildIndexSouthWest = 2;
constexpr size_t ChildIndexSouthEast = 3;

}  // namespace

namespace panwave {

StationaryWaveletPacketTree::StationaryWaveletPacketTree(
    size_t height, const Wavelet* wavelet, PaddingMode padding_mode)
    : WaveletPacketTreeTemplateBase(height, wavelet),
      padding_mode_(padding_mode) {}

void StationaryWaveletPacketTree::Decompose() { this->DecomposeNode(0); }

void StationaryWaveletPacketTree::DecomposeNode(size_t node) {
  if (this->IsLeaf(node)) {
    return;
  }

  const size_t nw_child = this->GetChild(node, ChildIndexNorthWest);
  const size_t ne_child = this->GetChild(node, ChildIndexNorthEast);
  const size_t sw_child = this->GetChild(node, ChildIndexSouthWest);
  const size_t se_child = this->GetChild(node, ChildIndexSouthEast);

  WaveletMath::Decompose(this->GetNodeData(node).signal,
                         this->wavelet_->lowpassDecompositionFilter_,
                         this->wavelet_->highpassDecompositionFilter_,
                         &this->GetNodeData(nw_child).signal,
                         &this->GetNodeData(sw_child).signal, DyadicMode::Even,
                         this->padding_mode_);

  WaveletMath::Decompose(this->GetNodeData(node).signal,
                         this->wavelet_->lowpassDecompositionFilter_,
                         this->wavelet_->highpassDecompositionFilter_,
                         &this->GetNodeData(ne_child).signal,
                         &this->GetNodeData(se_child).signal, DyadicMode::Odd,
                         this->padding_mode_);

  this->DecomposeNode(nw_child);
  this->DecomposeNode(ne_child);
  this->DecomposeNode(sw_child);
  this->DecomposeNode(se_child);
}

void StationaryWaveletPacketTree::ReconstructNode(size_t node) {
  if (this->IsLeaf(node)) {
    return;
  }

  const size_t nw_child = this->GetChild(node, ChildIndexNorthWest);
  const size_t ne_child = this->GetChild(node, ChildIndexNorthEast);
  const size_t sw_child = this->GetChild(node, ChildIndexSouthWest);
  const size_t se_child = this->GetChild(node, ChildIndexSouthEast);

  ReconstructNode(nw_child);
  ReconstructNode(ne_child);
  ReconstructNode(sw_child);
  ReconstructNode(se_child);

  DyadicMode dyad_mode = DyadicMode::Even;
  const std::vector<double>* child_signal = nullptr;
  const std::vector<double>* reconstruction_filter = nullptr;

  if (this->IsMarked(nw_child)) {
    assert(!this->IsMarked(sw_child) && !this->IsMarked(ne_child) &&
           !this->IsMarked(se_child));
    dyad_mode = DyadicMode::Even;
    child_signal = &this->GetNodeData(nw_child).signal;
    reconstruction_filter = &this->wavelet_->lowpassReconstructionFilter_;
  } else if (this->IsMarked(sw_child)) {
    assert(!this->IsMarked(nw_child) && !this->IsMarked(ne_child) &&
           !this->IsMarked(se_child));
    dyad_mode = DyadicMode::Even;
    child_signal = &this->GetNodeData(sw_child).signal;
    reconstruction_filter = &this->wavelet_->highpassReconstructionFilter_;
  } else if (this->IsMarked(ne_child)) {
    assert(!this->IsMarked(sw_child) && !this->IsMarked(nw_child) &&
           !this->IsMarked(se_child));
    dyad_mode = DyadicMode::Odd;
    child_signal = &this->GetNodeData(ne_child).signal;
    reconstruction_filter = &this->wavelet_->lowpassReconstructionFilter_;
  } else if (this->IsMarked(se_child)) {
    assert(!this->IsMarked(sw_child) && !this->IsMarked(ne_child) &&
           !this->IsMarked(nw_child));
    dyad_mode = DyadicMode::Odd;
    child_signal = &this->GetNodeData(se_child).signal;
    reconstruction_filter = &this->wavelet_->highpassReconstructionFilter_;
  }

  if (child_signal != nullptr) {
    this->SetMark(node);

    auto& data = this->GetNodeData(node);
    WaveletMath::Reconstruct(*child_signal, *reconstruction_filter,
                             &data.signal, data.signal.size(), dyad_mode,
                             this->padding_mode_);
  }
}

void StationaryWaveletPacketTree::ReconstructAccumulate(
    size_t leaf_node, std::vector<double>* accumulated_signal) {
  assert(this->IsLeaf(leaf_node));
  assert(accumulated_signal);

  this->Unmark();
  this->SetMark(leaf_node);

  this->ReconstructNode(0);

  assert(this->GetRootSignal().size() == accumulated_signal->size());

  std::transform(accumulated_signal->cbegin(), accumulated_signal->cend(),
                 this->GetRootSignal().cbegin(), accumulated_signal->begin(),
                 std::plus<>());
}

void StationaryWaveletPacketTree::Reconstruct(size_t level) {
  // If height is 1, we only have the root node so there's nothing to
  // reconstruct.
  if (this->GetHeight() == 1) {
    return;
  }

  const size_t leaf_count = this->GetLeafCount();
  const size_t level_count = this->GetWaveletLevelCount();
  const size_t first_leaf_index = this->GetFirstLeaf();

  assert(level < level_count);

  std::vector<double> reconstructed_signal;
  reconstructed_signal.resize(this->GetRootSignal().size());

  // First calculate the starting leaf index for the level.
  size_t starting_leaf = 0;
  size_t current_leaf_count = 4;
  size_t current_level_count = 2;
  while (current_leaf_count <= leaf_count) {
    const size_t remainder = level % current_level_count;

    if (remainder >= current_level_count / 2) {
      starting_leaf += current_leaf_count / 2;
    }

    current_leaf_count *= 4;
    current_level_count *= 2;
  }

  // Now use the starting leaf index to calculate the remaining leaves in the
  // row.
  for (size_t i = 0; i < level_count / 2; i++) {
    size_t current_leaf = starting_leaf;
    size_t current_multiplier = 4;
    size_t level_index = i;

    while (level_index != 0) {
      // Least significant bit is set.
      if ((level_index & 0x1U) != 0U) {
        current_leaf += current_multiplier;
      }

      // Shift to next most significant bit.
      level_index = level_index >> 1U;
      // Increase multiplier to next power of 4;
      current_multiplier *= 4;
    }

    // We only calculated half of the leaf indices (the even half)
    // but the odd halves immediately follow each even half so we
    // need to reconstruct current_leaf and current_leaf + 1.
    this->ReconstructAccumulate(first_leaf_index + current_leaf,
                                &reconstructed_signal);
    this->ReconstructAccumulate(first_leaf_index + current_leaf + 1,
                                &reconstructed_signal);
  }

  for (double& it : reconstructed_signal) {
    it /= static_cast<double>(level_count);
  }

  this->SetRootSignal(reconstructed_signal);
}

}  // namespace panwave
