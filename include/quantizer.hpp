#pragma once

#include "dct.hpp"
#include <array>
#include <vector>

using QuantizedBlock8x8 = std::array<int, 64>;

struct QuantizedChannelBlocks {
    int blocksPerRow = 0;
    int blocksPerCol = 0;
    std::vector<QuantizedBlock8x8> blocks;
};

struct QuantizedImageBlocks {
    QuantizedChannelBlocks yBlocks;
    QuantizedChannelBlocks cbBlocks;
    QuantizedChannelBlocks crBlocks;
};

class Quantizer {
public:
    static QuantizedBlock8x8 quantizeBlock(const DctBlock8x8& block,
                                           const std::array<int, 64>& table);

    static QuantizedChannelBlocks quantizeChannel(const DctChannelBlocks& channelBlocks,
                                                  const std::array<int, 64>& table);

    static QuantizedImageBlocks quantizeImage(const DctImageBlocks& imageBlocks,
                                              int qualityFactor);

    static std::array<int, 64> scaledLuminanceTable(int qualityFactor);
    static std::array<int, 64> scaledChrominanceTable(int qualityFactor);
};