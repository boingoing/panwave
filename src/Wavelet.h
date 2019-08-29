//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panwave contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#pragma once

#include <vector>
#include <cstdint>

namespace panwave {
    /**
     * Container for a set of wavelet filters.<br/>
     * Manually fill out the filters or use GetWaveletCoefficients to load
     * well-known wavelet filter values.
     * @see GetWaveletCoefficients
     */
    class Wavelet {
    public:
        enum class WaveletType : uint8_t {
            Daubechies = 1,
            Symlet,
            Coiflet
        };

        std::vector<double> LowpassDecompositionFilter;
        std::vector<double> HighpassDecompositionFilter;
        std::vector<double> LowpassReconstructionFilter;
        std::vector<double> HighpassReconstructionFilter;

    public:
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
         * @param p Wavelet p value to load well-known values. The p value must be
         *          within the supported range. Use GetWaveletMinimumP and
         *          GetWaveletMaximumP to find the range.
         * @see WaveletType
         * @see GetWaveletMinimumP
         * @see GetWaveletMaximumP
         */
        static void GetWaveletCoefficients(Wavelet* wavelet, WaveletType type, size_t p);

        /**
         * Return the minimum wavelet p value supported for well-known wavelet
         * filter coefficients of a type of wavelet.
         * @param type Wavelet type to get the minimum p value.
         * @see WaveletType
         * @see GetWaveletCoefficients
         */
        static size_t GetWaveletMinimumP(WaveletType type);

        /**
         * Return the maximum wavelet p value supported for well-known wavelet
         * filter coefficients of a type of wavelet.
         * @param type Wavelet type to get the maximum p value.
         * @see WaveletType
         * @see GetWaveletCoefficients
         */
        static size_t GetWaveletMaximumP(WaveletType type);
    };
}; // namespace panwave
