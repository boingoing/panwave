//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panwave contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for
// full license information.
//-------------------------------------------------------------------------------------------------------

#include "Wavelet.h"

#include <cassert>
#include <vector>

namespace {

// Coefficient Source: http://disp.ee.ntu.edu.tw/tutorial/WaveletTutorial.pdf

// TODO(boingoing): Ideally these initializer_lists should be marked constexpr but this doesn't seem to work in MSVC.
const std::initializer_list<double>
    daubechiesLowpassDecompositionCoefficients[] = {
        {},
        {},
        {-0.129409523, 0.224143868, 0.836516304, 0.482962913},
        {0.035226292, -0.085441274, -0.13501102, 0.459877502, 0.806891509,
         0.332670553},
        {-0.010597401785, 0.03288301166698, 0.03084138183599, -0.18703481171888,
         -0.02798376941698, 0.63088076792959, 0.71484657055254,
         0.23037781330886},
        {0.00333572528500, -0.01258075199902, -0.00624149021301,
         0.07757149384007, -0.03224486958503, -0.24229488706619,
         0.13842814590110, 0.72430852843857, 0.60382926979747,
         0.16010239797413},
        {-0.001077301, 0.004777258, 0.000553842, -0.031582039, 0.027522866,
         0.097501606, -0.129766868, -0.226264694, 0.315250352, 0.751133908,
         0.49462389, 0.111540743},
        {0.000353714, -0.001801641, 0.000429578, 0.012550999, -0.016574542,
         -0.038029937, 0.080612609, 0.071309219, -0.224036185, -0.143906004,
         0.469782287, 0.729132091, 0.396539319, 0.077852054},
        {-0.000117477, 0.000675449, -0.00039174, -0.004870353, 0.008746094,
         0.013981028, -0.044088254, -0.017369301, 0.128747427, 0.000472485,
         -0.284015543, -0.015829105, 0.585354684, 0.675630736, 0.312871591,
         0.054415842},
        {3.93E-05, -0.000251963, 0.000230386, 0.001847647, -0.004281504,
         -0.004723205, 0.022361662, 0.000250947, -0.067632829, 0.030725681,
         0.148540749, -0.096840783, -0.293273783, 0.133197386, 0.657288078,
         0.604823124, 0.243834675, 0.038077947},
        {-1.33E-05,    9.36E-05,     -0.000116467, -0.000685857, 0.001992405,
         0.001395352,  -0.010733175, 0.003606554,  0.033212674,  -0.029457537,
         -0.071394147, 0.093057365,  0.12736934,   -0.195946274, -0.249846424,
         0.281172344,  0.688459039,  0.527201189,  0.1881768,    0.026670058},
};

const std::initializer_list<double>
    daubechiesHighpassDecompositionCoefficients[] = {
        {},
        {},
        {-0.482962913, 0.836516304, -0.224143868, -0.129409523},
        {-0.332670553, 0.806891509, -0.459877502, -0.13501102, 0.085441274,
         0.035226292},
        {-0.23037781330886, 0.71484657055254, -0.63088076792959,
         -0.02798386941698, 0.18703481171888, 0.03084138183599,
         -0.03288301166698, -0.010597401785},
        {-0.16010239797413, 0.60382926979747, -0.72430852843857,
         0.13842814590110, 0.24229488706619, -0.03224486958503,
         -0.07757149384007, -0.00624149021301, 0.01258075199902,
         0.00333572528500},
        {-0.111540743, 0.49462389, -0.751133908, 0.315250352, 0.226264694,
         -0.129766868, -0.097501606, 0.027522866, 0.031582039, 0.000553842,
         -0.004777258, -0.001077301},
        {-0.077852054, 0.396539319, -0.729132091, 0.469782287, 0.143906004,
         -0.224036185, -0.071309219, 0.080612609, 0.038029937, -0.016574542,
         -0.012550999, 0.000429578, 0.001801641, 0.000353714},
        {-0.054415842, 0.312871591, -0.675630736, 0.585354684, 0.015829105,
         -0.284015543, -0.000472485, 0.128747427, 0.017369301, -0.044088254,
         -0.013981028, 0.008746094, 0.004870353, -0.00039174, -0.000675449,
         -0.000117477},
        {-0.038077947, 0.243834675, -0.604823124, 0.657288078, -0.133197386,
         -0.293273783, 0.096840783, 0.148540749, -0.030725681, -0.067632829,
         -0.000250947, 0.022361662, 0.004723205, -0.004281504, -0.001847647,
         0.000230386, 0.000251963, 3.93E-05},
        {-0.026670058, 0.1881768,   -0.527201189, 0.688459039,  -0.281172344,
         -0.249846424, 0.195946274, 0.12736934,   -0.093057365, -0.071394147,
         0.029457537,  0.033212674, -0.003606554, -0.010733175, -0.001395352,
         0.001992405,  0.000685857, -0.000116467, -9.36E-05,    -1.33E-05},
};

const std::initializer_list<double>
    daubechiesLowpassReconstructionCoefficients[] = {
        {},
        {},
        {0.482962913, 0.836516304, 0.224143868, -0.129409523},
        {0.332670553, 0.806891509, 0.459877502, -0.13501102, -0.085441274,
         0.035226292},
        {0.23037781330886, 0.71484657055254, 0.63088076792959,
         -0.02798376941698, -0.18703481171888, 0.03084138183599,
         0.03288301166698, -0.010597401785},
        {0.16010239797413, 0.60382926979747, 0.72430852843857, 0.13842814590110,
         -0.24229488706619, -0.03224486958503, 0.07757149384007,
         -0.00624149021301, -0.01258075199902, 0.00333572528500},
        {0.111540743, 0.49462389, 0.751133908, 0.315250352, -0.226264694,
         -0.129766868, 0.097501606, 0.027522866, -0.031582039, 0.000553842,
         0.004777258, -0.001077301},
        {0.077852054, 0.396539319, 0.729132091, 0.469782287, -0.143906004,
         -0.224036185, 0.071309219, 0.080612609, -0.038029937, -0.016574542,
         0.012550999, 0.000429578, -0.001801641, 0.000353714},
        {0.054415842, 0.312871591, 0.675630736, 0.585354684, -0.015829105,
         -0.284015543, 0.000472485, 0.128747427, -0.017369301, -0.044088254,
         0.013981028, 0.008746094, -0.004870353, -0.00039174, 0.000675449,
         -0.000117477},
        {0.038077947, 0.243834675, 0.604823124, 0.657288078, 0.133197386,
         -0.293273783, -0.096840783, 0.148540749, 0.030725681, -0.067632829,
         0.000250947, 0.022361662, -0.004723205, -0.004281504, 0.001847647,
         0.000230386, -0.000251963, 3.93E-05},
        {0.026670058,  0.1881768,    0.527201189,  0.688459039,  0.281172344,
         -0.249846424, -0.195946274, 0.12736934,   0.093057365,  -0.071394147,
         -0.029457537, 0.033212674,  0.003606554,  -0.010733175, 0.001395352,
         0.001992405,  -0.000685857, -0.000116467, 9.36E-05,     -1.33E-05},
};

const std::initializer_list<double>
    daubechiesHighpassReconstructionCoefficients[] = {
        {},
        {},
        {-0.129409523, -0.224143868, 0.836516304, -0.482962913},
        {0.035226292, 0.085441274, -0.13501102, -0.459877502, 0.806891509,
         -0.332670553},
        {-0.010597401785, -0.03288301166698, 0.03084138183599, 0.18703481171888,
         -0.02798386941698, -0.63088076792959, 0.71484657055254,
         -0.23037781330886},
        {0.00333572528500, 0.01258075199902, -0.00624149021301,
         -0.07757149384007, -0.03224486958503, 0.24229488706619,
         0.13842814590110, -0.72430852843857, 0.60382926979747,
         -0.16010239797413},
        {-0.001077301, -0.004777258, 0.000553842, 0.031582039, 0.027522866,
         -0.097501606, -0.129766868, 0.226264694, 0.315250352, -0.751133908,
         0.49462389, -0.111540743},
        {0.000353714, 0.001801641, 0.000429578, -0.012550999, -0.016574542,
         0.038029937, 0.080612609, -0.071309219, -0.224036185, 0.143906004,
         0.469782287, -0.729132091, 0.396539319, -0.077852054},
        {-0.000117477, -0.000675449, -0.00039174, 0.004870353, 0.008746094,
         -0.013981028, -0.044088254, 0.017369301, 0.128747427, -0.000472485,
         -0.284015543, 0.015829105, 0.585354684, -0.675630736, 0.312871591,
         -0.054415842},
        {3.93E-05, 0.000251963, 0.000230386, -0.001847647, -0.004281504,
         0.004723205, 0.022361662, -0.000250947, -0.067632829, -0.030725681,
         0.148540749, 0.096840783, -0.293273783, -0.133197386, 0.657288078,
         -0.604823124, 0.243834675, -0.038077947},
        {-1.33E-05,    -9.36E-05,    -0.000116467, 0.000685857, 0.001992405,
         -0.001395352, -0.010733175, -0.003606554, 0.033212674, 0.029457537,
         -0.071394147, -0.093057365, 0.12736934,   0.195946274, -0.249846424,
         -0.281172344, 0.688459039,  -0.527201189, 0.1881768,   -0.026670058},
};

const std::initializer_list<double>
    symletLowpassDecompositionCoefficients[] = {
        {},
        {},
        {-0.129409523, 0.224143868, 0.836516304, 0.482962913},
        {0.035226292, -0.085441274, -0.13501102, 0.459877502, 0.806891509,
         0.332670553},
        {-0.075765714789273, -0.029635527645999, 0.497618667632015,
         0.803738751805916, 0.297857795605277, -0.099219543576847,
         -0.012603967262038, 0.032223100604043},
        {0.027333068345078, 0.029519490925775, -0.039134249302383,
         0.199397533977394, 0.723407690402421, 0.633978963458212,
         0.016602105764522, -0.175328089908450, -0.021101834024759,
         0.019538882735287},
};

const std::initializer_list<double>
    symletHighpassDecompositionCoefficients[] = {
        {},
        {},
        {-0.482962913, 0.836516304, -0.224143868, -0.129409523},
        {-0.332670553, 0.806891509, -0.459877502, -0.13501102, 0.085441274,
         0.035226292},
        {-0.032223100604043, -0.012603967262038, 0.099219543576847,
         0.297857795605277, -0.803738751805916, 0.497618667632015,
         0.029635527645999, -0.075765714789273},
        {-0.019538882735287, -0.021101834024759, 0.175328089908450,
         0.016602105764522, -0.633978963458212, 0.723407690402421,
         -0.199397533977394, -0.039134249302383, -0.029519490925775,
         0.027333068345078},
};

const std::initializer_list<double>
    symletLowpassReconstructionCoefficients[] = {
        {},
        {},
        {0.482962913, 0.836516304, 0.224143868, -0.129409523},
        {0.332670553, 0.806891509, 0.459877502, -0.13501102, -0.085441274,
         0.035226292},
        {0.032223100604043, -0.012603967262038, -0.099219543576847,
         0.297857795605277, 0.803738751805916, 0.497618667632015,
         -0.029635527645999, -0.075765714789273},
        {0.019538882735287, -0.021101834024759, -0.175328089908450,
         0.016602105764522, 0.633978963458212, 0.723407690402421,
         0.199397533977394, -0.039134249302383, 0.029519490925775,
         0.027333068345078},
};

const std::initializer_list<double>
    symletHighpassReconstructionCoefficients[] = {
        {},
        {},
        {-0.129409523, -0.224143868, 0.836516304, -0.482962913},
        {0.035226292, 0.085441274, -0.13501102, -0.459877502, 0.806891509,
         -0.332670553},
        {-0.075765714789273, 0.029635527645999, 0.497618667632015,
         -0.803738751805916, 0.297857795605277, 0.099219543576847,
         -0.012603967262038, -0.032223100604043},
        {0.027333068345078, -0.029519490925775, -0.039134249302383,
         -0.199397533977394, 0.723407690402421, -0.633978963458212,
         0.016602105764522, 0.175328089908450, -0.021101834024759,
         -0.019538882735287},
};

const std::initializer_list<double>
    coifletLowpassDecompositionCoefficients[] = {
        {},
        {-0.015655728, -0.07273262, 0.384864847, 0.85257202, 0.337897662,
         -0.07273262},
        {-0.000720549, -0.001823209, 0.005611435, 0.023680172, -0.059434419,
         -0.076488599, 0.417005184, 0.812723635, 0.386110067, -0.067372555,
         -0.041464937, 0.016387336},
        {-3.46E-05, -7.10E-05, 0.000466217, 0.001117519, -0.002574518,
         -0.009007976, 0.015880545, 0.034555028, -0.082301927, -0.071799822,
         0.428483476, 0.793777223, 0.405176902, -0.06112339, -0.065771911,
         0.023452696, 0.007782596, -0.003793513},
        {-1.78E-06,    -3.26E-06,    3.12E-05,     6.23E-05,     -0.000259975,
         -0.000589021, 0.001266562,  0.003751436,  -0.005658287, -0.015211732,
         0.025082262,  0.039334427,  -0.096220442, -0.066627474, 0.434386056,
         0.782238931,  0.415308407,  -0.056077313, -0.0812667,   0.0266823,
         0.016068944,  -0.007346166, -0.001629492, 0.000892314},
        {-9.52E-08,    -1.67E-07,    2.06E-06,     3.73E-06,     -2.13E-05,
         -4.13E-05,    0.000140541,  0.00030226,   -0.000638131, -0.001662864,
         0.002433373,  0.006764185,  -0.009164231, -0.019761779, 0.032683574,
         0.041289209,  -0.105574209, -0.062035964, 0.437991626,  0.774289604,
         0.421566207,  -0.052043163, -0.091920011, 0.028168029,  0.023408157,
         -0.010131118, -0.004159359, 0.002178236,  0.00035859,   -0.000212081},
};

const std::initializer_list<double>
    coifletHighpassDecompositionCoefficients[] = {
        {},
        {0.07273262, 0.337897662, -0.85257202, 0.384864847, 0.07273262,
         -0.015655728},
        {-0.016387336, -0.041464937, 0.067372555, 0.386110067, -0.812723635,
         0.417005184, 0.076488599, -0.059434419, -0.023680172, 0.005611435,
         0.001823209, -0.000720549},
        {0.003793513, 0.007782596, -0.023452696, -0.065771911, 0.06112339,
         0.405176902, -0.793777223, 0.428483476, 0.071799822, -0.082301927,
         -0.034555028, 0.015880545, 0.009007976, -0.002574518, -0.001117519,
         0.000466217, 7.10E-05, -3.46E-05},
        {-0.000892314, -0.001629492, 0.007346166,  0.016068944,  -0.0266823,
         -0.0812667,   0.056077313,  0.415308407,  -0.782238931, 0.434386056,
         0.066627474,  -0.096220442, -0.039334427, 0.025082262,  0.015211732,
         -0.005658287, -3.75E-03,    1.27E-03,     0.000589021,  -0.000259975,
         -6.23E-05,    3.12E-05,     3.26E-06,     -1.78E-06},
        {0.000212081,  0.00035859,   -0.002178236, -0.004159359, 0.010131118,
         0.023408157,  -0.028168029, -0.091920011, 0.052043163,  0.421566207,
         -0.774289604, 0.437991626,  0.062035964,  -0.105574209, -0.041289209,
         0.032683574,  1.98E-02,     -9.16E-03,    -0.006764185, 0.002433373,
         1.66E-03,     -6.38E-04,    -3.02E-04,    1.41E-04,     4.13E-05,
         -2.13E-05,    -3.73E-06,    2.06E-06,     1.67E-07,     -9.52E-08},
};

const std::initializer_list<double>
    coifletLowpassReconstructionCoefficients[] = {
        {},
        {-0.07273262, 0.337897662, 0.85257202, 0.384864847, -0.07273262,
         -0.015655728},
        {0.016387336, -0.041464937, -0.067372555, 0.386110067, 0.812723635,
         0.417005184, -0.076488599, -0.059434419, 0.023680172, 0.005611435,
         -0.001823209, -0.000720549},
        {-0.003793513, 0.007782596, 0.023452696, -0.065771911, -0.06112339,
         0.405176902, 0.793777223, 0.428483476, -0.071799822, -0.082301927,
         0.034555028, 0.015880545, -0.009007976, -0.002574518, 0.001117519,
         0.000466217, -7.10E-05, -3.46E-05},
        {0.000892314,  -0.001629492, -0.007346166, 0.016068944,  0.0266823,
         -0.0812667,   -0.056077313, 0.415308407,  0.782238931,  0.434386056,
         -0.066627474, -0.096220442, 0.039334427,  0.025082262,  -0.015211732,
         -0.005658287, 3.75E-03,     1.27E-03,     -0.000589021, -0.000259975,
         6.23E-05,     3.12E-05,     -3.26E-06,    -1.78E-06},
        {-0.000212081, 0.00035859,  0.002178236,  -0.004159359, -0.010131118,
         0.023408157,  0.028168029, -0.091920011, -0.052043163, 0.421566207,
         0.774289604,  0.437991626, -0.062035964, -0.105574209, 0.041289209,
         0.032683574,  -1.98E-02,   -9.16E-03,    0.006764185,  0.002433373,
         -1.66E-03,    -6.38E-04,   3.02E-04,     1.41E-04,     -4.13E-05,
         -2.13E-05,    3.73E-06,    2.06E-06,     -1.67E-07,    -9.52E-08},
};

const std::initializer_list<double>
    coifletHighpassReconstructionCoefficients[] = {
        {},
        {-0.015655728, 0.07273262, 0.384864847, -0.85257202, 0.337897662,
         0.07273262},
        {-0.000720549, 0.001823209, 0.005611435, -0.023680172, -0.059434419,
         0.076488599, 0.417005184, -0.812723635, 0.386110067, 0.067372555,
         -0.041464937, -0.016387336},
        {-3.46E-05, 7.10E-05, 0.000466217, -0.001117519, -0.002574518,
         0.009007976, 0.015880545, -0.034555028, -0.082301927, 0.071799822,
         0.428483476, -0.793777223, 0.405176902, 0.06112339, -0.065771911,
         -0.023452696, 0.007782596, 0.003793513},
        {-1.78E-06,    3.26E-06,     3.12E-05,     -6.23E-05,    -0.000259975,
         0.000589021,  0.001266562,  -0.003751436, -0.005658287, 0.015211732,
         0.025082262,  -0.039334427, -0.096220442, 0.066627474,  0.434386056,
         -0.782238931, 0.415308407,  0.056077313,  -0.0812667,   -0.0266823,
         0.016068944,  0.007346166,  -0.001629492, -0.000892314},
        {-9.52E-08,    1.67E-07,     2.06E-06,     -3.73E-06,    -2.13E-05,
         4.13E-05,     0.000140541,  -0.00030226,  -0.000638131, 0.001662864,
         0.002433373,  -0.006764185, -0.009164231, 0.019761779,  0.032683574,
         -0.041289209, -0.105574209, 0.062035964,  0.437991626,  -0.774289604,
         0.421566207,  0.052043163,  -0.091920011, -0.028168029, 0.023408157,
         0.010131118,  -0.004159359, -0.002178236, 0.00035859,   0.000212081},
};

constexpr size_t daubechiesMinIndex = 2;
constexpr size_t daubechiesMaxIndex = 10;

constexpr size_t symletMinIndex = 2;
constexpr size_t symletMaxIndex = 5;

constexpr size_t coifletMinIndex = 1;
constexpr size_t coifletMaxIndex = 5;

void DaubechiesCoefficients(std::vector<double>* LoD, std::vector<double>* HiD,
                            std::vector<double>* LoR, std::vector<double>* HiR,
                            size_t index) {
  assert(index >= daubechiesMinIndex && index <= daubechiesMaxIndex);
  LoD->assign(daubechiesLowpassDecompositionCoefficients[index]);
  HiD->assign(daubechiesHighpassDecompositionCoefficients[index]);
  LoR->assign(daubechiesLowpassReconstructionCoefficients[index]);
  HiR->assign(daubechiesHighpassReconstructionCoefficients[index]);
}

void SymletCoefficients(std::vector<double>* LoD, std::vector<double>* HiD,
                        std::vector<double>* LoR, std::vector<double>* HiR,
                        size_t index) {
  assert(index >= symletMinIndex && index <= symletMaxIndex);
  LoD->assign(symletLowpassDecompositionCoefficients[index]);
  HiD->assign(symletHighpassDecompositionCoefficients[index]);
  LoR->assign(symletLowpassReconstructionCoefficients[index]);
  HiR->assign(symletHighpassReconstructionCoefficients[index]);
}

void CoifletCoefficients(std::vector<double>* LoD, std::vector<double>* HiD,
                         std::vector<double>* LoR, std::vector<double>* HiR,
                         size_t index) {
  assert(index >= coifletMinIndex && index <= coifletMaxIndex);
  LoD->assign(coifletLowpassDecompositionCoefficients[index]);
  HiD->assign(coifletHighpassDecompositionCoefficients[index]);
  LoR->assign(coifletLowpassReconstructionCoefficients[index]);
  HiR->assign(coifletHighpassReconstructionCoefficients[index]);
}

}  // namespace

namespace panwave {

// static
size_t Wavelet::GetWaveletMinimumP(WaveletType type) {
  switch (type) {
    case WaveletType::Daubechies:
      return daubechiesMinIndex;
    case WaveletType::Symlet:
      return symletMinIndex;
    case WaveletType::Coiflet:
      return coifletMinIndex;
    default:
      assert(false);
      return 0;
  }
}

// static
size_t Wavelet::GetWaveletMaximumP(WaveletType type) {
  switch (type) {
    case WaveletType::Daubechies:
      return daubechiesMaxIndex;
    case WaveletType::Symlet:
      return symletMaxIndex;
    case WaveletType::Coiflet:
      return coifletMaxIndex;
    default:
      assert(false);
      return 0;
  }
}

// static
void Wavelet::GetWaveletCoefficients(Wavelet* wavelet, WaveletType type,
                                     size_t vanishing_moment) {
  switch (type) {
    case WaveletType::Daubechies:
      DaubechiesCoefficients(&wavelet->lowpassDecompositionFilter_,
                             &wavelet->highpassDecompositionFilter_,
                             &wavelet->lowpassReconstructionFilter_,
                             &wavelet->highpassReconstructionFilter_,
                             vanishing_moment);
      break;
    case WaveletType::Symlet:
      SymletCoefficients(&wavelet->lowpassDecompositionFilter_,
                         &wavelet->highpassDecompositionFilter_,
                         &wavelet->lowpassReconstructionFilter_,
                         &wavelet->highpassReconstructionFilter_,
                         vanishing_moment);
      break;
    case WaveletType::Coiflet:
      CoifletCoefficients(&wavelet->lowpassDecompositionFilter_,
                          &wavelet->highpassDecompositionFilter_,
                          &wavelet->lowpassReconstructionFilter_,
                          &wavelet->highpassReconstructionFilter_,
                          vanishing_moment);
      break;
    default:
      assert(false);
      break;
  }
}

size_t Wavelet::Length() const {
  assert(this->highpassDecompositionFilter_.size() ==
         this->highpassReconstructionFilter_.size());
  assert(this->highpassDecompositionFilter_.size() ==
         this->lowpassDecompositionFilter_.size());
  assert(this->highpassDecompositionFilter_.size() ==
         this->lowpassReconstructionFilter_.size());

  return this->highpassDecompositionFilter_.size();
}

}  // namespace panwave
