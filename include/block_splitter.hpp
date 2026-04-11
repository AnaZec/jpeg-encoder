#pragma once

#include "color_converter.hpp"
#include <array>
#include <vector>

using Block8x8 = std::array<uint8_t, 64>;

struct ChannelBlocks {
    int blocksPerRow = 0;
    int blocksPerCol = 0;
    std::vector<Block8x8> blocks;
};

struct ImageBlocks {
    ChannelBlocks yBlocks;
    ChannelBlocks cbBlocks;
    ChannelBlocks crBlocks;
};

class BlockSplitter {
public:
    static ChannelBlocks splitChannelIntoBlocks(const std::vector<uint8_t>& channel,
                                                int width,
                                                int height);

    static ImageBlocks splitImageIntoBlocks(const YCbCrImage& image);
};