#include "dct_test.hpp"
#include "dct.hpp"

#include <array>
#include <cmath>
#include <iostream>

namespace {
bool nearlyEqual(double a, double b, double tolerance) {
    return std::abs(a - b) <= tolerance;
}
}

bool DCTTest::run() {
    const double tolerance = 1e-9;

    Block8x8 inputBlock{};
    inputBlock.fill(128);

    DctBlock8x8 expected{};
    expected.fill(0.0);

    DctBlock8x8 actual = DCT::forwardDCT(inputBlock);

    bool passed = true;

    for (std::size_t i = 0; i < 64; ++i) {
        if (!nearlyEqual(actual[i], expected[i], tolerance)) {
            std::cerr << "DCT test failed at coefficient " << i
                      << ": expected " << expected[i]
                      << ", got " << actual[i] << "\n";
            passed = false;
        }
    }

    if (passed) {
        std::cout << "DCT known-block test passed.\n";
    }

    return passed;
}