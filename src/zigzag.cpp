#include "zigzag.hpp"

namespace {

constexpr std::array<int, 64> kZigZagIndices = {
     0,  1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

} // namespace

ZigZagBlock ZigZag::reorderBlock(const QuantizedBlock8x8& block) {
    ZigZagBlock result{};

    for (std::size_t i = 0; i < 64; ++i) {
        result[i] = block[kZigZagIndices[i]];
    }

    return result;
}

ZigZagChannelBlocks ZigZag::reorderChannel(const QuantizedChannelBlocks& channelBlocks) {
    ZigZagChannelBlocks result;
    result.blocksPerRow = channelBlocks.blocksPerRow;
    result.blocksPerCol = channelBlocks.blocksPerCol;
    result.blocks.reserve(channelBlocks.blocks.size());

    for (const auto& block : channelBlocks.blocks) {
        result.blocks.push_back(reorderBlock(block));
    }

    return result;
}

ZigZagImageBlocks ZigZag::reorderImage(const QuantizedImageBlocks& imageBlocks) {
    ZigZagImageBlocks result;
    result.yBlocks = reorderChannel(imageBlocks.yBlocks);
    result.cbBlocks = reorderChannel(imageBlocks.cbBlocks);
    result.crBlocks = reorderChannel(imageBlocks.crBlocks);
    return result;
}