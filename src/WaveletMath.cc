//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panwave contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for
// full license information.
//-------------------------------------------------------------------------------------------------------

#include "WaveletMath.h"

#include <algorithm>
#include <cassert>

namespace panwave {

void WaveletMath::Pad(const std::vector<double>& data,
                      std::vector<double>* extended_data, size_t pad_left,
                      size_t pad_right, PaddingMode padding_mode) {
  assert(extended_data);

  extended_data->clear();
  extended_data->resize(data.size() + pad_left + pad_right);

  // First fill in the middle part of the extended data.
  // This is the existing content of data.
  std::copy(data.cbegin(), data.cend(), extended_data->begin() + pad_left);

  if (padding_mode == PaddingMode::Symmetric) {
    size_t index = 0;
    if (pad_left >= data.size()) {
      for (size_t i = 0; i <= pad_left - data.size(); i++) {
        extended_data->operator[](index++) = data.back();
      }
    }
    // Elements in the left-padding section.
    for (size_t i = index; i < pad_left; i++) {
      extended_data->operator[](index++) = data[pad_left - i];
    }
    index = extended_data->size() - 1;
    if (pad_right >= data.size()) {
      for (size_t i = 0; i <= pad_right - data.size(); i++) {
        extended_data->operator[](index--) = data.front();
      }
    }
    // Elements in the right-padding section.
    for (size_t i = pad_right - ((extended_data->size() - 1) - index); i > 0;
         i--) {
      extended_data->operator[](index--) = data[data.size() - 1 - i];
    }
  } else {
    // If padding_mode is PaddingMode::Zeroes we have nothing to do.
    // The default value for double is 0.
    assert(padding_mode == PaddingMode::Zeroes);
  }
}

void WaveletMath::Convolve(const std::vector<double>& data,
                           const std::vector<double>& coeffs,
                           std::vector<double>* result) {
  assert(result);
  assert(data.size() >= coeffs.size());
  assert(!coeffs.empty());

  result->resize(data.size() - (coeffs.size() - 1));

  for (size_t i = 0; i < result->size(); i++) {
    double val = 0.0;

    for (size_t j = 0; j < coeffs.size(); j++) {
      val +=
          data[i + j] * coeffs[coeffs.size() - j - 1];
    }

    result->operator[](i) = val;
  }
}

void WaveletMath::DyadicDownsample(const std::vector<double>& data,
                                   std::vector<double>* data_downsampled,
                                   DyadicMode dyadic_mode) {
  assert(data_downsampled);

  data_downsampled->clear();

  for (size_t i = dyadic_mode == DyadicMode::Even ? 0U : 1U; i < data.size();
       i += 2) {
    data_downsampled->push_back(data[i]);
  }
}

void WaveletMath::DyadicUpsample(const std::vector<double>& data,
                                 std::vector<double>* data_upsampled,
                                 DyadicMode dyadic_mode) {
  assert(!data.empty());
  assert(data_upsampled);

  size_t source_index = 0;
  const size_t new_size =
      data.size() * 2 + (dyadic_mode == DyadicMode::Even ? 1 : -1);

  data_upsampled->clear();
  data_upsampled->resize(new_size);

  for (size_t i = dyadic_mode == DyadicMode::Even ? 1U : 0U;
       i < data_upsampled->size(); i += 2) {
    data_upsampled->operator[](i) = data[source_index++];
  }
}

void WaveletMath::Decompose(const std::vector<double>& data,
                            const std::vector<double>& lowpass_filter_coeffs,
                            const std::vector<double>& highpass_filter_coeffs,
                            std::vector<double>* approx_coeffs,
                            std::vector<double>* details_coeffs,
                            DyadicMode dyadic_mode, PaddingMode padding_mode) {
  assert(approx_coeffs);
  assert(details_coeffs);
  assert(lowpass_filter_coeffs.size() == highpass_filter_coeffs.size());
  assert(!lowpass_filter_coeffs.empty());

  std::vector<double> data_padded;
  std::vector<double> low_pass_data;
  std::vector<double> high_pass_data;
  const auto filter_size = lowpass_filter_coeffs.size();

  Pad(data, &data_padded, filter_size - 1, filter_size - 1, padding_mode);

  Convolve(data_padded, lowpass_filter_coeffs, &low_pass_data);
  Convolve(data_padded, highpass_filter_coeffs, &high_pass_data);

  DyadicDownsample(low_pass_data, approx_coeffs, dyadic_mode);
  DyadicDownsample(high_pass_data, details_coeffs, dyadic_mode);
}

void WaveletMath::Reconstruct(const std::vector<double>& coeffs,
                              const std::vector<double>& reconstruction_coeffs,
                              std::vector<double>* data, size_t data_size,
                              DyadicMode dyadic_mode,
                              PaddingMode padding_mode) {
  assert(data);
  assert(reconstruction_coeffs.size() > 2);

  std::vector<double> upsampled_coeffs;
  std::vector<double> upsampled_coeffs_padded;
  std::vector<double> data_wide;
  const auto filter_size = reconstruction_coeffs.size();

  DyadicUpsample(coeffs, &upsampled_coeffs, dyadic_mode);
  Pad(upsampled_coeffs, &upsampled_coeffs_padded, filter_size - 1,
      filter_size - 1, padding_mode);
  Convolve(upsampled_coeffs_padded, reconstruction_coeffs, &data_wide);

  const size_t dyad_shift = dyadic_mode == DyadicMode::Even ? 0U : 2U;
  auto begin = data_wide.cbegin() + (filter_size - dyad_shift);
  data->resize(data_size);
  std::copy_n(begin, data_size, data->begin());
}

}  // namespace panwave
