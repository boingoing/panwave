//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panwave contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for
// full license information.
//-------------------------------------------------------------------------------------------------------

#ifndef WAVELETMATH_H
#define WAVELETMATH_H

#include <cstdint>
#include <vector>

namespace panwave {
class Wavelet;
}  // namespace panwave

namespace panwave {

/**
 * The mode used when dyadically upsampling or downsampling.
 * @see DyadicUpsample
 * @see DyadicDownsample
 */
enum class DyadicMode : uint8_t { Even = 0, Odd };

/**
 * When padding data in WaveletMath::Pad, this mode controls what value
 * is used for the padding elements in the extended data vector.
 * @see Pad
 */
enum class PaddingMode : uint8_t { Zeroes = 0, Symmetric };

/**
 * A container for static methods useful to compute wavelet math functions.
 * This is not meant to be a complete wavelet solution, it exists to allow
 * the narrow set of wavelet math functions required to compute wavelet
 * packet trees.
 * @see WaveletPacketTree
 * @see StationaryWaveletPacketTree
 */
class WaveletMath {
 public:
  /**
   * Decompose a signal into approximation and details coefficients.<br/>
   * Uses wavelet to get the decomposition highpass and lowpass filters.
   * @param data The signal data we wish to decompose.
   * @param wavelet Wavelet we will use to get the highpass and lowpass
   *                decomposition filters.
   * @param approx_coeffs Destination approximation coefficients. Any
   *                      existing contents will be overwritten.
   * @param details_coeffs Destination details coefficients. Any
   *                      existing contents will be overwritten.
   * @param dyadic_mode Mode we should use when dyadically downsampling.
   *                    (default: Odd)
   * @param padding_mode Padding mode we should use when padding the
   *                     signal data. (Default: Zeroes)
   */
  static void Decompose(const std::vector<double>* data, const Wavelet* wavelet,
                        std::vector<double>* approx_coeffs,
                        std::vector<double>* details_coeffs,
                        DyadicMode dyadic_mode, PaddingMode padding_mode);

  /**
   * Reconstruct a signal from approximation or details coefficients.
   * @param coeffs Either the approximation or details coefficients
   *               produced during a decomposition.
   * @param reconstruction_coeffs Should be a lowpass or highpass
   *                              reconstruction filter with size equal
   *                              to the length of wavelet.
   * @param data Destination vector for the reconstructed signal. Any
   *             existing contents will be erased.
   * @param wavelet Wavelet we used to do the decomposition.
   * @param data_size Size of the reconstructed signal.
   * @param dyadic_mode Mode we should use when dyadically upsampling.
   *                    (default: Odd)
   * @param padding_mode Padding mode we should use when padding the
   *                     coefficient data. (Default: Zeroes)
   */
  static void Reconstruct(const std::vector<double>* coeffs,
                          const std::vector<double>* reconstruction_coeffs,
                          std::vector<double>* data, const Wavelet* wavelet,
                          size_t data_size, DyadicMode dyadic_mode,
                          PaddingMode padding_mode);

  /**
   * Dyadically upsample a data signal.<br/>
   * All of the original values from data are included in the upsampled
   * data but there are zeroes separating each value. The dyadic mode
   * determines which indices in the upsampled vector contain zeroes.<br/>
   * If dyadic mode is Even, every even index in the upsampled data will
   * contain a zero.<br/>
   * If dyadic mode is Odd, every odd index in the upsampled data will
   * contain a zero.<br/>
   * The upsampled data has size double the original data's size plus one
   * in even dyadic mode or minus one in odd dyadic mode.
   * @param data The original data signal we want to upsample.
   * @param data_upsampled The upsampled data will be written to this
   *                       vector. Existing contents will be erased.
   * @param dyadic_mode Mode we should use to dyadically upsample.
   *                    (default: Odd)
   * @see DyadicMode
   */
  static void DyadicUpsample(const std::vector<double>* data,
                             std::vector<double>* data_upsampled,
                             DyadicMode dyadic_mode);

  /**
   * Dyadically downsample a data signal.<br/>
   * Half of the original values are included in the downsampled data.
   * The dyadic mode controls which elements from the original data are
   * taken into the downsampled data.<br/>
   * If dyadic mode is Even, every even index value in data is copied into
   * the downsampled data. Odd index values are left out.<br/>
   * If dyadic mode is Odd, every odd index value in data is copied into
   * the downsampled data. Even index values are left out.<br/>
   * The downsampled data has size half the original data's size. Even
   * dyadic mode rounds up, Odd dyadic mode rounds down in case original
   * data's size is not even.
   * @param data The original data signal we want to downsample.
   * @param data_downsampled The downsampled data will be written to this
   *                         vector. Existing contents will be erased.
   * @param dyadic_mode Mode we should use to dyadically downsample.
   *                    (default: Odd)
   * @see DyadicMode
   */
  static void DyadicDownsample(const std::vector<double>* data,
                               std::vector<double>* data_downsampled,
                               DyadicMode dyadic_mode);

  /**
   * Perform a convolution.
   * @param data The input signal to convolve.
   * @param coeffs The filter coefficients.
   * @param result The destination for our convolved signal data. Any
   *               values in the vector will be overwritten.
   */
  static void Convolve(const std::vector<double>* data,
                       const std::vector<double>* coeffs,
                       std::vector<double>* result);

  /**
   * Pad a vector by inserting elements on the right and left.<br/>
   * If padding_mode is Zeroes, the inserted elements are all zero.<br/>
   * If padding_mode is Symmetric, the inserted elements are the values
   * closest to the beginning or end but flipped symmetrically around
   * either the first or last element. If the requested pad length is
   * longer than data size, the last or first element of data is used
   * for all the elements which would overflow data.<br/>
   * For example, [1,2,3] symmetrically padded by 3 on the left and
   * right produces [3,3,2,1,2,3,2,1,1]
   * @param data The original data we want to pad.
   * @param extended_data The destination for our padded data. It will
   *                      have length equal to pad_left + pad_right +
   *                      data->size(). Any existing values will be
   *                      overwritten.
   * @param pad_left The number of elements to pad on the left of data.
   *                 Can be zero.
   * @param pad_right The number of elements to pad on the right of
   *                  data. Can be zero.
   * @param padding_mode Padding mode we should use to insert padding
   *                     elements.
   * @see PaddingMode
   */
  static void Pad(const std::vector<double>* data,
                  std::vector<double>* extended_data, size_t pad_left,
                  size_t pad_right, PaddingMode padding_mode);
};

}  // namespace panwave

#endif  // WAVELETMATH_H
