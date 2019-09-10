//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panwave contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include <vector>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <cstring>
#include <sstream>

#include "WaveletMath.h"
#include "WaveletPacketTree.h"
#include "StationaryWaveletPacketTree.h"
#include "WaveletPacketTreeBase.h"

using namespace panwave;

void print(const std::vector<double>* v) {
    for (size_t i = 0; i < v->size(); i++) {
        std::cout << v->operator[](i) << ' ';
    }
}

void compare(const std::vector<double>* left, const std::vector<double>* right) {
    if (left->size() != right->size()) {
        std::cout << "Failed size check. Expected: " << left->size() << " Actual: " << right->size() << std::endl;
    }
    for (size_t i = 0; i < left->size(); i++) {
        if (fabs(left->operator[](i) - right->operator[](i)) > 0.001) {
            std::cout << "Failed vector values." << std::endl << "Expected: { ";
            print(left);
            std::cout << " }" << std::endl << "Actual: { ";
            print(right);
            std::cout << "}" << std::endl;
            return;
        }
    }
}

void TestWPT(WaveletPacketTreeBase* tree, size_t height, const std::vector<double>* signal, const Wavelet* wavelet, bool verify) {
    tree->SetRootSignal(signal);
    tree->Decompose();

    std::vector<double> reconstructed_signal;
    reconstructed_signal.resize(signal->size());

    for (size_t i = 0; i < tree->GetWaveletLevelCount(); i++) {
        tree->Reconstruct(i);
        std::transform(reconstructed_signal.cbegin(), reconstructed_signal.cend(), tree->GetRootSignal()->cbegin(), reconstructed_signal.begin(), std::plus<double>());
    }

    if (verify) {
        compare(signal, &reconstructed_signal);
    }
    std::cout << "Pass" << std::endl;
}

void TestSWPT(size_t height, const std::vector<double>* signal, const Wavelet* wavelet) {
    std::cout << "Testing StationaryWaveletPacketTree height = " << height << std::endl;
    StationaryWaveletPacketTree tree(height, wavelet);
    TestWPT(&tree, height, signal, wavelet, true);
}

void TestWPT(size_t height, const std::vector<double>* signal, const Wavelet* wavelet) {
    std::cout << "Testing WaveletPacketTree height = " << height << std::endl;
    WaveletPacketTree tree(height, wavelet);
    TestWPT(&tree, height, signal, wavelet, true);
}

void TestWPTs(size_t max_height, const std::vector<double>* signal, const Wavelet* wavelet) {
    for (size_t i = 0; i < max_height; i++) {
        TestWPT(i + 1, signal, wavelet);
    }
    for (size_t i = 0; i < 7; i++) {
        TestSWPT(i + 1, signal, wavelet);
    }
}

void TestWavelet(Wavelet::WaveletType type, size_t max_height, const std::vector<double>* signal) {
    size_t min = Wavelet::GetWaveletMinimumP(type);
    // We support coiflet up to p=5 but these are more lossy so the verification will fail
    size_t max = type != Wavelet::WaveletType::Coiflet ? Wavelet::GetWaveletMaximumP(type) : 2;
    Wavelet w;

    for (size_t i = min; i <= max; i++) {
        Wavelet::GetWaveletCoefficients(&w, type, i);
        std::cout << "Testing with p=" << i << std::endl;
        TestWPTs(max_height, signal, &w);
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

int main(int argc, const char** argv) {
    std::vector<double> signal;
    signal.resize(500);
    for (size_t i = 0; i < signal.size(); i++) {
        signal[i] = i;
    }
    TestWavelets(10, &signal);

    TestDyadicUp({1,2,3,4,5,6,7,8,9,10}, {0,1,0,2,0,3,0,4,0,5,0,6,0,7,0,8,0,9,0,10,0},DyadicMode::Even);
    TestDyadicUp({1,2,3,4,5,6,7,8,9,10}, {1,0,2,0,3,0,4,0,5,0,6,0,7,0,8,0,9,0,10},DyadicMode::Odd);
    TestDyadicUp({1,2,3,4,5,6,7,8,9}, {1,0,2,0,3,0,4,0,5,0,6,0,7,0,8,0,9},DyadicMode::Odd);
    TestDyadicUp({1,2,3,4,5,6,7,8,9}, {0,1,0,2,0,3,0,4,0,5,0,6,0,7,0,8,0,9,0},DyadicMode::Even);
    TestDyadicUp({1,2}, {0,1,0,2,0},DyadicMode::Even);
    TestDyadicUp({1,2}, {1,0,2},DyadicMode::Odd);
    TestDyadicUp({1}, {0,1,0},DyadicMode::Even);
    TestDyadicUp({1}, {1},DyadicMode::Odd);

    TestDyadicDown({1,2,3,4,5,6,7,8,9}, {2,4,6,8}, DyadicMode::Odd);
    TestDyadicDown({1,2,3,4,5,6,7,8,9}, {1,3,5,7,9}, DyadicMode::Even);
    TestDyadicDown({1,2,3,4,5,6,7,8,9,10}, {2,4,6,8,10}, DyadicMode::Odd);
    TestDyadicDown({1,2,3,4,5,6,7,8,9,10}, {1,3,5,7,9}, DyadicMode::Even);

    TestPad({1,2,3,4,5}, {5,5,5,4,3,2,1,2,3,4,5,4,3,2,1,1,1}, 6, 6, PaddingMode::Symmetric);
    TestPad({1,2,3,4,5}, {5,5,4,3,2,1,2,3,4,5,4,3,2,1,1}, 5, 5, PaddingMode::Symmetric);
    TestPad({1,2,3,4,5}, {0,0,0,0,0,0,1,2,3,4,5,0,0,0,0,0,0}, 6, 6, PaddingMode::Zeroes);

    return 0;
}
