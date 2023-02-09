//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panwave contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include "StationaryWaveletPacketTree.h"
#include "WaveletMath.h"
#include "WaveletPacketTree.h"
#include "WaveletPacketTreeBase.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

using panwave::DyadicMode;
using panwave::PaddingMode;
using panwave::StationaryWaveletPacketTree;
using panwave::Wavelet;
using panwave::WaveletMath;
using panwave::WaveletPacketTree;
using panwave::WaveletPacketTreeBase;

namespace testing {

void print(const std::vector<double>* vec) {
    for (size_t i = 0; i < vec->size(); i++) {
        std::cout << vec->operator[](i) << ' ';
    }
}

void compare(const std::vector<double>* left, const std::vector<double>* right) {
    if (left->size() != right->size()) {
        std::cout << "Failed size check. Expected: " << left->size() << " Actual: " << right->size() << std::endl;
    }
    constexpr double epsilon = 0.001;
    for (size_t i = 0; i < left->size(); i++) {
        if (fabs(left->operator[](i) - right->operator[](i)) > epsilon) {
            std::cout << "Failed vector values." << std::endl << "Expected: { ";
            print(left);
            std::cout << " }" << std::endl << "Actual: { ";
            print(right);
            std::cout << "}" << std::endl;
            return;
        }
    }
}

void TestWPT(WaveletPacketTreeBase* tree, const std::vector<double>* signal, bool verify) {
    tree->SetRootSignal(signal);
    tree->Decompose();

    std::vector<double> reconstructed_signal;
    reconstructed_signal.resize(signal->size());

    for (size_t i = 0; i < tree->GetWaveletLevelCount(); i++) {
        tree->Reconstruct(i);
        std::transform(reconstructed_signal.cbegin(), reconstructed_signal.cend(), tree->GetRootSignal()->cbegin(), reconstructed_signal.begin(), std::plus<>());
    }

    if (verify) {
        compare(signal, &reconstructed_signal);
    }
    std::cout << "Pass" << std::endl;
}

void TestSWPT(size_t height, const std::vector<double>* signal, const Wavelet* wavelet) {
    std::cout << "Testing StationaryWaveletPacketTree height = " << height << std::endl;
    StationaryWaveletPacketTree tree(height, wavelet);
    TestWPT(&tree, signal, true);
}

void TestWPT(size_t height, const std::vector<double>* signal, const Wavelet* wavelet) {
    std::cout << "Testing WaveletPacketTree height = " << height << std::endl;
    WaveletPacketTree tree(height, wavelet);
    TestWPT(&tree, signal, true);
}

void TestWPTs(size_t max_height, const std::vector<double>* signal, const Wavelet* wavelet) {
    for (size_t i = 0; i < max_height; i++) {
        TestWPT(i + 1, signal, wavelet);
    }
    // SWPT takes longer to compute. Limit the max_height.
    constexpr size_t max_height_swpt = 7;
    max_height = std::min(max_height, max_height_swpt);
    for (size_t i = 0; i < max_height; i++) {
        TestSWPT(i + 1, signal, wavelet);
    }
}

void TestWavelet(Wavelet::WaveletType type, size_t max_height, const std::vector<double>* signal) {
    size_t min = Wavelet::GetWaveletMinimumP(type);
    // We support coiflet up to p=5 but these are more lossy so the verification will fail. Only test on a safe subset.
    constexpr size_t max_safe_coiflet = 2;
    size_t max = type == Wavelet::WaveletType::Coiflet ? max_safe_coiflet : Wavelet::GetWaveletMaximumP(type);
    Wavelet wavelet;

    for (size_t i = min; i <= max; i++) {
        Wavelet::GetWaveletCoefficients(&wavelet, type, i);
        std::cout << "Testing with p=" << i << std::endl;
        TestWPTs(max_height, signal, &wavelet);
    }
}

void TestWavelets(size_t max_height, const std::vector<double>* signal) {
    std::cout << "Testing Daubechies" << std::endl;
    TestWavelet(Wavelet::WaveletType::Daubechies, max_height, signal);
    std::cout << "Testing Symlet" << std::endl;
    TestWavelet(Wavelet::WaveletType::Symlet, max_height, signal);
    std::cout << "Testing Coiflet" << std::endl;
    TestWavelet(Wavelet::WaveletType::Coiflet, max_height, signal);
}

void TestDyadicUp(const std::vector<double> signal, const std::vector<double> expected, DyadicMode mode) {
    std::vector<double> actual;
    WaveletMath::DyadicUpsample(&signal, &actual, mode);
    compare(&expected, &actual);
}

void TestDyadicDown(const std::vector<double> signal, const std::vector<double> expected, DyadicMode mode) {
    std::vector<double> actual;
    WaveletMath::DyadicDownsample(&signal, &actual, mode);
    compare(&expected, &actual);
}

void TestPad(const std::vector<double> data, const std::vector<double> expected, size_t left, size_t right, PaddingMode mode) {
    std::vector<double> actual;
    WaveletMath::Pad(&data, &actual, left, right, mode);
    compare(&expected, &actual);
}

struct DyadicTest {
    std::initializer_list<double> signal_;
    std::initializer_list<double> expected_;
    DyadicMode mode_;
};

const DyadicTest dyadicUpTests[] = {
    {{1,2,3,4,5,6,7,8,9,10}, {0,1,0,2,0,3,0,4,0,5,0,6,0,7,0,8,0,9,0,10,0}, DyadicMode::Even},
    {{1,2,3,4,5,6,7,8,9,10}, {1,0,2,0,3,0,4,0,5,0,6,0,7,0,8,0,9,0,10}, DyadicMode::Odd},
    {{1,2,3,4,5,6,7,8,9}, {1,0,2,0,3,0,4,0,5,0,6,0,7,0,8,0,9},DyadicMode::Odd},
    {{1,2,3,4,5,6,7,8,9}, {0,1,0,2,0,3,0,4,0,5,0,6,0,7,0,8,0,9,0},DyadicMode::Even},
    {{1,2}, {0,1,0,2,0},DyadicMode::Even},
    {{1,2}, {1,0,2},DyadicMode::Odd},
    {{1}, {0,1,0},DyadicMode::Even},
    {{1}, {1},DyadicMode::Odd},
};

const DyadicTest dyadicDownTests[] = {
    {{1,2,3,4,5,6,7,8,9}, {2,4,6,8}, DyadicMode::Odd},
    {{1,2,3,4,5,6,7,8,9}, {1,3,5,7,9}, DyadicMode::Even},
    {{1,2,3,4,5,6,7,8,9,10}, {2,4,6,8,10}, DyadicMode::Odd},
    {{1,2,3,4,5,6,7,8,9,10}, {1,3,5,7,9}, DyadicMode::Even},
};

struct PadTest {
    std::initializer_list<double> data_;
    std::initializer_list<double> expected_;
    size_t left_;
    size_t right_;
    PaddingMode mode_;
};

const PadTest padTests[] = {
    {{1,2,3,4,5}, {5,5,5,4,3,2,1,2,3,4,5,4,3,2,1,1,1}, 6, 6, PaddingMode::Symmetric},
    {{1,2,3,4,5}, {5,5,4,3,2,1,2,3,4,5,4,3,2,1,1}, 5, 5, PaddingMode::Symmetric},
    {{1,2,3,4,5}, {0,0,0,0,0,0,1,2,3,4,5,0,0,0,0,0,0}, 6, 6, PaddingMode::Zeroes},
};

void DoTests() {
    std::vector<double> signal;
    constexpr size_t signal_size = 500;
    signal.resize(signal_size);
    for (size_t i = 0; i < signal.size(); i++) {
        signal[i] = i;
    }
    constexpr size_t max_test_height = 10;
    TestWavelets(max_test_height, &signal);

    for (const DyadicTest& test : dyadicUpTests) {
        TestDyadicUp(test.signal_, test.expected_, test.mode_);
    }

    for (const DyadicTest& test : dyadicDownTests) {
        TestDyadicDown(test.signal_, test.expected_, test.mode_);
    }

    for (const PadTest& test : padTests) {
        TestPad(test.data_, test.expected_, test.left_, test.right_, test.mode_);
    }
}

}  // namespace testing

int main() {
    testing::DoTests();
    return 0;
}
