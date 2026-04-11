#include "dct_test.hpp"

#include <iostream>

int main() {
    try {
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