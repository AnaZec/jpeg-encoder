#pragma once

#include <array>
#include <cstdint>
#include <vector>

struct JpegHuffmanTable {
    std::array<uint8_t, 16> codeLengthCounts{};
    std::vector<uint8_t> symbols;
};

class HuffmanTables {
public:
    static const JpegHuffmanTable& luminanceDCTable();
    static const JpegHuffmanTable& luminanceACTable();
    static const JpegHuffmanTable& chrominanceDCTable();
    static const JpegHuffmanTable& chrominanceACTable();
};