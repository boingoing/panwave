//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panwave contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for
// full license information.
//-------------------------------------------------------------------------------------------------------

#ifndef WAVELETPACKETTREE_H
#define WAVELETPACKETTREE_H

#include "WaveletMath.h"
#include "WaveletPacketTreeTemplateBase.h"

namespace panwave {

class Wavelet;

/**
 * A conventional wavelet packet tree class.<br/>
 * This is a binary tree, each non-leaf node has two children.<br/>
 * During decomposition, each node is decomposed into details and
 * approximation coefficients. The approximation coefficients are stored in
 * the left (0th) child while the details coefficients are stored in the
 * right (1st) child.
 */
class WaveletPacketTree : public WaveletPacketTreeTemplateBase<2> {
 public:
  /**
   * Construct a WaveletPacketTree instance.<br/>
   * Root signal is initially unset. Set it before calling Decompose.
   * @param height Height of the tree. A tree with only one root node
   *               has height of 1.
   * @param wavelet Wavelet object used during decomposition /
   *                reconstruction.
   * @param dyadic_mode Which mode we should use when dyadically
   *                    upsampling / downsampling when performing
   *                    convolutions. (default: Odd)
   * @param padding_mode How we should pad the signal data during
   *                     decomposition / reconstruction. (default: Zeroes)
   * @see Wavelet
   * @see Decompose
   * @see Reconstruct
   */
  WaveletPacketTree(size_t height, const Wavelet* wavelet,
                    DyadicMode dyadic_mode = DyadicMode::Odd,
                    PaddingMode padding_mode = PaddingMode::Zeroes);
  ~WaveletPacketTree() override = default;

  void Decompose() override;
  void Reconstruct(size_t level) override;

 protected:
  void IsolateLevel(size_t level);

  void DecomposeNode(size_t node);
  void ReconstructNode(size_t node);

 private:
  DyadicMode dyadic_mode_;
  PaddingMode padding_mode_;
};

}  // namespace panwave

#endif  // WAVELETPACKETTREE_H
