#include "huffman_tables.hpp"

#include <iostream>

int main() {
    try {
        const auto& lumaDC = HuffmanTables::luminanceDCTable();
        const auto& lumaAC = HuffmanTables::luminanceACTable();
        const auto& chromaDC = HuffmanTables::chrominanceDCTable();
        const auto& chromaAC = HuffmanTables::chrominanceACTable();

        std::cout << "Luma DC symbol count: " << lumaDC.symbols.size() << "\n";
        std::cout << "Luma AC symbol count: " << lumaAC.symbols.size() << "\n";
        std::cout << "Chroma DC symbol count: " << chromaDC.symbols.size() << "\n";
        std::cout << "Chroma AC symbol count: " << chromaAC.symbols.size() << "\n";

        std::cout << "Standard JPEG Huffman tables loaded successfully.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}