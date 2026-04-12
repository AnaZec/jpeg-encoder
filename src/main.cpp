#include "zigzag_test.hpp"

#include <iostream>

int main() {
    try {
        const bool passed = ZigZagTest::run();

        if (!passed) {
            std::cerr << "Zigzag verification failed.\n";
            return 1;
        }

        std::cout << "Zigzag verification succeeded.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}