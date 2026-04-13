#pragma once

#include <array>
#include <cstdint>
#include <unordered_map>
#include <vector>

struct JpegHuffmanTable {
    std::array<uint8_t, 16> codeLengthCounts{};
    std::vector<uint8_t> symbols;
};

struct HuffmanCode {
    uint16_t code = 0;
    uint8_t length = 0;
};

using HuffmanCodeMap = std::unordered_map<uint8_t, HuffmanCode>;

class HuffmanTables {
public:
    static const JpegHuffmanTable& luminanceDCTable();
    static const JpegHuffmanTable& luminanceACTable();
    static const JpegHuffmanTable& chrominanceDCTable();
    static const JpegHuffmanTable& chrominanceACTable();

    static HuffmanCodeMap luminanceDCCodes();
    static HuffmanCodeMap luminanceACCodes();
    static HuffmanCodeMap chrominanceDCCodes();
    static HuffmanCodeMap chrominanceACCodes();

private:
    static HuffmanCodeMap buildCanonicalCodes(const JpegHuffmanTable& table);
};