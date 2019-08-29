//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panwave contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include <cassert>

#include "WaveletMath.h"

using namespace panwave;

void WaveletMath::Pad(const std::vector<double>* data,
                      std::vector<double>* extended_data,
                      size_t pad_left,
                      size_t pad_right,
                      PaddingMode padding_mode) {
    assert(data);
    assert(extended_data);

    extended_data->clear();
    extended_data->resize(data->size() + pad_left + pad_right);

    // First fill in the middle part of the extended data.
    // This is the existing content of data.
    for (size_t i = 0; i < data->size(); i++) {
        extended_data->operator[](pad_left + i) = data->operator[](i);
    }

    if (padding_mode == PaddingMode::Symmetric) {
        size_t index = 0;
        if (pad_left >= data->size()) {
            for (size_t i = 0; i <= pad_left - data->size(); i++) {
                extended_data->operator[](index++) = data->operator[](data->size() - 1);
            }
        }
        // Elements in the left-padding section.
        for (size_t i = index; i < pad_left; i++) {
            extended_data->operator[](index++) = data->operator[](pad_left - i);
        }
        index = extended_data->size() - 1;
        if (pad_right >= data->size()) {
            for (size_t i = 0; i <= pad_right - data->size(); i++) {
                extended_data->operator[](index--) = data->operator[](0);
            }
        }
        // Elements in the right-padding section.
        for (size_t i = pad_right - ((extended_data->size() - 1) - index); i > 0; i--) {
            extended_data->operator[](index--) = data->operator[](data->size() - 1 - i);
        }
    } else {
        // If padding_mode is PaddingMode::Zeroes we have nothing to do.
        // The default value for double is 0.
        assert(padding_mode == PaddingMode::Zeroes);
    }
}

void WaveletMath::Convolve(const std::vector<double>* data,
                           const std::vector<double>* coeffs,
                           std::vector<double>* result) {
    assert(data);
    assert(coeffs);
    assert(result);
    assert(data->size() >= coeffs->size());
    assert(coeffs->size() > 0);

    result->resize(data->size() - (coeffs->size() - 1));

    for (size_t i = 0; i < result->size(); i++) {
        double val = 0.0;

        for (size_t j = 0; j < coeffs->size(); j++) {
            val += data->operator[](i + j) * coeffs->operator[](coeffs->size() - j - 1);
        }

        result->operator[](i) = val;
    }
}

void WaveletMath::DyadicDownsample(const std::vector<double>* data,
                                   std::vector<double>* data_downsampled,
                                   DyadicMode dyadic_mode) {
    assert(data);
    assert(data_downsampled);

    data_downsampled->clear();

    size_t i = dyadic_mode == DyadicMode::Even ? 0 : 1;

    for (; i < data->size(); i+=2) {
        data_downsampled->push_back(data->operator[](i));
    }
}

void WaveletMath::DyadicUpsample(const std::vector<double>* data,
                                 std::vector<double>* data_upsampled,
                                 DyadicMode dyadic_mode) {
    assert(data);
    assert(data->size() > 0);
    assert(data_upsampled);

    size_t j = 0;
    size_t i = dyadic_mode == DyadicMode::Even ? 1 : 0;
    size_t new_size = data->size() * 2 + (dyadic_mode == DyadicMode::Even ? 1 : -1);

    data_upsampled->clear();
    data_upsampled->resize(new_size);

    for (; i < data_upsampled->size(); i += 2) {
        data_upsampled->operator[](i) = data->operator[](j++);
    }
}

void WaveletMath::Decompose(const std::vector<double>* data,
                            const Wavelet* wavelet,
                            std::vector<double>* approx_coeffs,
                            std::vector<double>* details_coeffs,
                            DyadicMode dyadic_mode,
                            PaddingMode padding_mode) {
    assert(data);
    assert(approx_coeffs);
    assert(details_coeffs);
    assert(wavelet);
    assert(wavelet->Length() > 0);

    std::vector<double> data_padded;
    std::vector<double> low_pass_data;
    std::vector<double> high_pass_data;

    Pad(data, &data_padded, wavelet->Length() - 1, wavelet->Length() - 1, padding_mode);

    Convolve(&data_padded, &wavelet->LowpassDecompositionFilter, &low_pass_data);
    Convolve(&data_padded, &wavelet->HighpassDecompositionFilter, &high_pass_data);

    DyadicDownsample(&low_pass_data, approx_coeffs, dyadic_mode);
    DyadicDownsample(&high_pass_data, details_coeffs, dyadic_mode);
}

void WaveletMath::Reconstruct(const std::vector<double>* coeffs,
                              const std::vector<double>* reconstruction_coeffs,
                              std::vector<double>* data,
                              const Wavelet* wavelet,
                              size_t data_size,
                              DyadicMode dyadic_mode,
                              PaddingMode padding_mode) {
    assert(coeffs);
    assert(reconstruction_coeffs);
    assert(data);
    assert(wavelet);
    assert(wavelet->Length() > 2);
    assert(wavelet->Length() == reconstruction_coeffs->size());

    std::vector<double> upsampled_coeffs;
    std::vector<double> upsampled_coeffs_padded;
    std::vector<double> data_wide;

    DyadicUpsample(coeffs, &upsampled_coeffs, dyadic_mode);
    Pad(&upsampled_coeffs, &upsampled_coeffs_padded, wavelet->Length() - 1, wavelet->Length() - 1, padding_mode);
    Convolve(&upsampled_coeffs_padded, reconstruction_coeffs, &data_wide);

    size_t dyad_shift = dyadic_mode == DyadicMode::Even ? 0 : 2;
    data->resize(data_size);
    auto begin = data_wide.cbegin() + wavelet->Length() - dyad_shift;
    data->assign(begin, begin + data_size);
}
