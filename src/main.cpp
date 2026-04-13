#include "huffman_tables.hpp"

#include <iostream>
#include <string>

namespace {

bool check(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << "\n";
        return false;
    }
    return true;
}

bool validateTable(const std::string& name,
                   const JpegHuffmanTable& table,
                   const HuffmanCodeMap& codes,
                   std::size_t expectedSymbolCount) {
    bool passed = true;

    std::size_t countedSymbols = 0;
    for (uint8_t count : table.codeLengthCounts) {
        countedSymbols += count;
    }

    passed &= check(countedSymbols == expectedSymbolCount,
                    name + " symbol count from code lengths matches expected count");

    passed &= check(table.symbols.size() == expectedSymbolCount,
                    name + " symbol vector size matches expected count");

    passed &= check(codes.size() == expectedSymbolCount,
                    name + " canonical code lookup size matches expected count");

    for (uint8_t symbol : table.symbols) {
        auto it = codes.find(symbol);
        passed &= check(it != codes.end(),
                        name + " contains lookup entry for symbol " + std::to_string(symbol));

        if (it != codes.end()) {
            passed &= check(it->second.length >= 1 && it->second.length <= 16,
                            name + " code length is in valid JPEG range for symbol " + std::to_string(symbol));
        }
    }

    return passed;
}

} // namespace

int main() {
    try {
        const auto& lumaDCTable = HuffmanTables::luminanceDCTable();
        const auto& lumaACTable = HuffmanTables::luminanceACTable();
        const auto& chromaDCTable = HuffmanTables::chrominanceDCTable();
        const auto& chromaACTable = HuffmanTables::chrominanceACTable();

        const HuffmanCodeMap lumaDCCodes = HuffmanTables::luminanceDCCodes();
        const HuffmanCodeMap lumaACCodes = HuffmanTables::luminanceACCodes();
        const HuffmanCodeMap chromaDCCodes = HuffmanTables::chrominanceDCCodes();
        const HuffmanCodeMap chromaACCodes = HuffmanTables::chrominanceACCodes();

        bool passed = true;

        passed &= validateTable("Luminance DC", lumaDCTable, lumaDCCodes, 12);
        passed &= validateTable("Luminance AC", lumaACTable, lumaACCodes, 162);
        passed &= validateTable("Chrominance DC", chromaDCTable, chromaDCCodes, 12);
        passed &= validateTable("Chrominance AC", chromaACTable, chromaACCodes, 162);

        // Specific lookup checks proving the maps are usable for DC categories and AC symbols
        passed &= check(lumaDCCodes.find(0) != lumaDCCodes.end(),
                        "Luminance DC lookup contains category 0");
        passed &= check(lumaDCCodes.find(11) != lumaDCCodes.end(),
                        "Luminance DC lookup contains category 11");

        passed &= check(chromaDCCodes.find(0) != chromaDCCodes.end(),
                        "Chrominance DC lookup contains category 0");
        passed &= check(chromaDCCodes.find(11) != chromaDCCodes.end(),
                        "Chrominance DC lookup contains category 11");

        passed &= check(lumaACCodes.find(0x00) != lumaACCodes.end(),
                        "Luminance AC lookup contains EOB symbol 0x00");
        passed &= check(lumaACCodes.find(0xF0) != lumaACCodes.end(),
                        "Luminance AC lookup contains ZRL symbol 0xF0");

        passed &= check(chromaACCodes.find(0x00) != chromaACCodes.end(),
                        "Chrominance AC lookup contains EOB symbol 0x00");
        passed &= check(chromaACCodes.find(0xF0) != chromaACCodes.end(),
                        "Chrominance AC lookup contains ZRL symbol 0xF0");

        if (!passed) {
            std::cerr << "Canonical Huffman code generation verification failed.\n";
            return 1;
        }

        std::cout << "Canonical Huffman code generation verification passed.\n";
        std::cout << "Luma DC codes: " << lumaDCCodes.size() << "\n";
        std::cout << "Luma AC codes: " << lumaACCodes.size() << "\n";
        std::cout << "Chroma DC codes: " << chromaDCCodes.size() << "\n";
        std::cout << "Chroma AC codes: " << chromaACCodes.size() << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}