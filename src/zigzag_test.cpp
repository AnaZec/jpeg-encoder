#include "zigzag_test.hpp"
#include "zigzag.hpp"

#include <iostream>

bool ZigZagTest::run() {
    QuantizedBlock8x8 inputBlock{};

    // Fill row-major with 0..63
    for (int i = 0; i < 64; ++i) {
        inputBlock[i] = i;
    }

    const ZigZagBlock actual = ZigZag::reorderBlock(inputBlock);

    const ZigZagBlock expected = {
         0,  1,  8, 16,  9,  2,  3, 10,
        17, 24, 32, 25, 18, 11,  4,  5,
        12, 19, 26, 33, 40, 48, 41, 34,
        27, 20, 13,  6,  7, 14, 21, 28,
        35, 42, 49, 56, 57, 50, 43, 36,
        29, 22, 15, 23, 30, 37, 44, 51,
        58, 59, 52, 45, 38, 31, 39, 46,
        53, 60, 61, 54, 47, 55, 62, 63
    };

    bool passed = true;

    for (int i = 0; i < 64; ++i) {
        if (actual[i] != expected[i]) {
            std::cerr << "Zigzag test failed at index " << i
                      << ": expected " << expected[i]
                      << ", got " << actual[i] << "\n";
            passed = false;
        }
    }

    if (passed) {
        std::cout << "Zigzag ordering test passed.\n";
    }

    return passed;
}