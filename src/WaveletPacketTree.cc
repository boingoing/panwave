//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panwave contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for
// full license information.
//-------------------------------------------------------------------------------------------------------

#include "WaveletPacketTree.h"

#include <cassert>

#include "Wavelet.h"
#include "WaveletPacketTreeTemplateBase.h"

namespace {

constexpr size_t ChildIndexLeft = 0;
constexpr size_t ChildIndexRight = 1;

}  // namespace

namespace panwave {

WaveletPacketTree::WaveletPacketTree(size_t height, const Wavelet* wavelet,
                                     DyadicMode dyadic_mode,
                                     PaddingMode padding_mode)
    : WaveletPacketTreeTemplateBase(height, wavelet),
      dyadic_mode_(dyadic_mode),
      padding_mode_(padding_mode) {}

void WaveletPacketTree::Decompose() { this->DecomposeNode(0); }

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
  if (this->IsLeaf(node)) {
    return;
  }

  const size_t left = this->GetChild(node, ChildIndexLeft);
  const size_t right = this->GetChild(node, ChildIndexRight);

  WaveletMath::Decompose(&this->GetNodeData(node).signal, this->wavelet_,
                         &this->GetNodeData(left).signal,
                         &this->GetNodeData(right).signal, this->dyadic_mode_,
                         this->padding_mode_);

  this->DecomposeNode(left);
  this->DecomposeNode(right);
}

void WaveletPacketTree::ReconstructNode(size_t node) {
  if (this->IsLeaf(node)) {
    return;
  }

  const size_t left = this->GetChild(node, ChildIndexLeft);
  const size_t right = this->GetChild(node, ChildIndexRight);

  ReconstructNode(left);
  ReconstructNode(right);

  const std::vector<double>* child_signal = nullptr;
  const std::vector<double>* reconstruction_filter = nullptr;

  if (this->IsMarked(left)) {
    assert(!this->IsMarked(right));
    child_signal = &this->GetNodeData(left).signal;
    reconstruction_filter = &this->wavelet_->lowpassReconstructionFilter_;
  } else if (this->IsMarked(right)) {
    assert(!this->IsMarked(left));
    child_signal = &this->GetNodeData(right).signal;
    reconstruction_filter = &this->wavelet_->highpassReconstructionFilter_;
  }

  if (child_signal != nullptr) {
    this->SetMark(node);

    auto& data = this->GetNodeData(node);
    WaveletMath::Reconstruct(child_signal, reconstruction_filter,
                             &data.signal, this->wavelet_,
                             data.signal.size(),
                             this->dyadic_mode_, this->padding_mode_);
  }
}

}  // namespace panwave
