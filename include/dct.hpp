#pragma once

#include <array>
#include <vector>
#include <cstdint>

// 8x8 spatial block (input)
using Block8x8 = std::array<uint8_t, 64>;

// 8x8 frequency block (output)
using DctBlock8x8 = std::array<double, 64>;

struct ChannelBlocks {
    int blocksPerRow = 0;
    int blocksPerCol = 0;
    std::vector<Block8x8> blocks;
};

struct DctChannelBlocks {
    int blocksPerRow = 0;
    int blocksPerCol = 0;
    std::vector<DctBlock8x8> blocks;
};

struct ImageBlocks {
    ChannelBlocks yBlocks;
    ChannelBlocks cbBlocks;
    ChannelBlocks crBlocks;
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