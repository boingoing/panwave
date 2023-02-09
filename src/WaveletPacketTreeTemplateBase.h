//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panwave contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for
// full license information.
//-------------------------------------------------------------------------------------------------------

#ifndef WAVELETPACKETTREETEMPLATEBASE_H
#define WAVELETPACKETTREETEMPLATEBASE_H

#include <vector>

#include "Tree.h"
#include "WaveletPacketTreeBase.h"

namespace panwave {
/**
 * A templated base class from which specialized wavelet packet tree
 * implementations can derive.<br/>
 * Template argument |k| is the number of children per node.
 */
template <size_t k>
class WaveletPacketTreeTemplateBase
    : public Tree<WaveletPacketTreeBase::WaveletPacketTreeNodeData, k>,
      public WaveletPacketTreeBase {
 public:
  WaveletPacketTreeTemplateBase(size_t height, const Wavelet* wavelet)
      : Tree<WaveletPacketTreeNodeData, k>(height), WaveletPacketTreeBase(), wavelet_(wavelet) {}

  void SetRootSignal(const std::vector<double>* signal) override {
    assert(signal);

    auto data = this->GetNodeData(0);
    data.signal.assign(signal->cbegin(), signal->cend());
  }

  const std::vector<double>* GetRootSignal() override {
    return &this->GetNodeData(0).signal;
  }

  size_t GetWaveletLevelCount() const override {
    return static_cast<size_t>(std::pow(2, this->GetHeight() - 1));
  }

 protected:
  const Wavelet* wavelet_;
};

}  // namespace panwave

#endif  // WAVELETPACKETTREETEMPLATEBASE_H
