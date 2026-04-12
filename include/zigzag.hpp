#pragma once

#include "quantizer.hpp"
#include <array>
#include <vector>

using ZigZagBlock = std::array<int, 64>;

struct ZigZagChannelBlocks {
    int blocksPerRow = 0;
    int blocksPerCol = 0;
    std::vector<ZigZagBlock> blocks;
};

struct ZigZagImageBlocks {
    ZigZagChannelBlocks yBlocks;
    ZigZagChannelBlocks cbBlocks;
    ZigZagChannelBlocks crBlocks;
};

class ZigZag {
public:
    static ZigZagBlock reorderBlock(const QuantizedBlock8x8& block);
    static ZigZagChannelBlocks reorderChannel(const QuantizedChannelBlocks& channelBlocks);
    static ZigZagImageBlocks reorderImage(const QuantizedImageBlocks& imageBlocks);
};
