#include "dct_test.hpp"

#include <iostream>

int main() {
    try {
        // Temporary test harness for verifying DCT module correctness
        // Confirms DCT is independently usable and produces expected results
        const bool passed = DCTTest::run();

        if (!passed) {
            std::cerr << "DCT verification failed.\n";
            return 1;
        }

        std::cout << "DCT verification succeeded.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}