//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panwave contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for
// full license information.
//-------------------------------------------------------------------------------------------------------

#ifndef WAVELETPACKETTREEBASE_H
#define WAVELETPACKETTREEBASE_H

#include <vector>

#include "Tree.h"

namespace panwave {

/**
 * Base class for all wavelet packet tree specialization types.<br/>
 * This abstract class is an interface to hold methods common to
 * various wavelet packet trees.
 */
class WaveletPacketTreeBase {
 public:
  WaveletPacketTreeBase(const WaveletPacketTreeBase&) = delete;
  WaveletPacketTreeBase(const WaveletPacketTreeBase&&) = delete;
  WaveletPacketTreeBase& operator=(const WaveletPacketTreeBase&) = delete;
  WaveletPacketTreeBase& operator=(const WaveletPacketTreeBase&&) = delete;

  WaveletPacketTreeBase() = default;
  virtual ~WaveletPacketTreeBase() = default;

  /**
   * This struct is just a container used to hold the signal data for each
   * node in the wavelet packet tree.
   */
  struct WaveletPacketTreeNodeData {
    std::vector<double> signal;
  };

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
  virtual void SetRootSignal(const std::vector<double>& signal) = 0;

  /**
   * Get a read-only view of the root node signal data.
   * @see Reconstruct
   */
  virtual const std::vector<double>& GetRootSignal() = 0;

  /**
   * Get the number of wavelet levels this tree is capable of
   * isolating and reconstructing.
   */
  virtual size_t GetWaveletLevelCount() const = 0;
};

}  // namespace panwave

#endif  // WAVELETPACKETTREEBASE_H
