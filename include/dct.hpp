#pragma once

#include "block_splitter.hpp"
#include <array>
#include <vector>

using DctBlock8x8 = std::array<double, 64>;

struct DctChannelBlocks {
    int blocksPerRow = 0;
    int blocksPerCol = 0;
    std::vector<DctBlock8x8> blocks;
};

struct DctImageBlocks {
    DctChannelBlocks yBlocks;
    DctChannelBlocks cbBlocks;
    DctChannelBlocks crBlocks;
};

class DCT {
public:
    static DctBlock8x8 forwardDCT(const Block8x8& inputBlock);
    static DctChannelBlocks applyToChannel(const ChannelBlocks& channelBlocks);
    static DctImageBlocks applyToImage(const ImageBlocks& imageBlocks);
};