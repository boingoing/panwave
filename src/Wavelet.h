//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panwave contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for
// full license information.
//-------------------------------------------------------------------------------------------------------

#ifndef WAVELET_H
#define WAVELET_H

#include <cstdint>
#include <vector>

namespace panwave {

/**
 * Container for a set of wavelet filters.<br/>
 * Manually fill out the filters or use GetWaveletCoefficients to load
 * well-known wavelet filter values.
 * @see GetWaveletCoefficients
 */
class Wavelet {
 public:
  enum class WaveletType : uint8_t { Daubechies = 1, Symlet, Coiflet };

  /**
   * The length of the wavelet. This is equivalent to the length of each
   * wavelet filter. We expect all wavelet filters to be of the same
   * length.
   */
  size_t Length() const;

  /**
   * Load the wavelet filter coefficients for a well-known wavelet.
   * @param wavelet Destination wavelet instance. Filter coefficients will be
   *                overwritten with well-known values.
   * @param type Wavelet type to load well-knwon values.
   * @param vanishing_moment Wavelet vanishing_moment value to load well-known
   * values. The vanishing_moment value must be within the supported range. Use
   *          GetWaveletMinimumP and GetWaveletMaximumP to find the range.
   * @see WaveletType
   * @see GetWaveletMinimumP
   * @see GetWaveletMaximumP
   */
  static void GetWaveletCoefficients(Wavelet* wavelet, WaveletType type,
                                     size_t vanishing_moment);

  /**
   * Return the minimum wavelet vanishing_moment value supported for well-known
   * wavelet filter coefficients of a type of wavelet.
   * @param type Wavelet type to get the minimum vanishing_moment value.
   * @see WaveletType
   * @see GetWaveletCoefficients
   */
  static size_t GetWaveletMinimumP(WaveletType type);

  /**
   * Return the maximum wavelet vanishing_moment value supported for well-known
   * wavelet filter coefficients of a type of wavelet.
   * @param type Wavelet type to get the maximum vanishing_moment value.
   * @see WaveletType
   * @see GetWaveletCoefficients
   */
  static size_t GetWaveletMaximumP(WaveletType type);

  std::vector<double> lowpassDecompositionFilter_;
  std::vector<double> highpassDecompositionFilter_;
  std::vector<double> lowpassReconstructionFilter_;
  std::vector<double> highpassReconstructionFilter_;
};

}  // namespace panwave

#endif  // WAVELET_H
