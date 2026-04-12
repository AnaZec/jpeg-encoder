#include "quantizer.hpp"

#include <iostream>
#include <string>

void printFirstEight(const std::string& label, const std::array<int, 64>& table) {
    std::cout << label;
    for (int i = 0; i < 8; ++i) {
        std::cout << table[i] << " ";
    }
    std::cout << "\n";
}

int main() {
    try {
        const int qualities[] = {1, 10, 50, 90, 100};

        for (int quality : qualities) {
            const auto lumTable = Quantizer::scaledLuminanceTable(quality);
            const auto chromaTable = Quantizer::scaledChrominanceTable(quality);

            std::cout << "Quality factor: " << quality << "\n";
            printFirstEight("  Luminance first 8:   ", lumTable);
            printFirstEight("  Chrominance first 8: ", chromaTable);
            std::cout << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}