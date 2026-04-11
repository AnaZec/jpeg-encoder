#include "block_splitter.hpp"

#include <stdexcept>

ChannelBlocks BlockSplitter::splitChannelIntoBlocks(const std::vector<uint8_t>& channel,
                                                    int width,
                                                    int height) {
    if (width <= 0 || height <= 0) {
        throw std::runtime_error("Invalid channel dimensions for block splitting");
    }

    if (width % 8 != 0 || height % 8 != 0) {
        throw std::runtime_error("Channel dimensions must be multiples of 8");
    }

    if (static_cast<int>(channel.size()) != width * height) {
        throw std::runtime_error("Channel size does not match dimensions");
    }

    ChannelBlocks result;
    result.blocksPerRow = width / 8;
    result.blocksPerCol = height / 8;
    result.blocks.reserve(result.blocksPerRow * result.blocksPerCol);

    for (int blockY = 0; blockY < height; blockY += 8) {
        for (int blockX = 0; blockX < width; blockX += 8) {
            Block8x8 block{};

            for (int y = 0; y < 8; ++y) {
                for (int x = 0; x < 8; ++x) {
                    const int imageX = blockX + x;
                    const int imageY = blockY + y;

                    block[y * 8 + x] = channel[imageY * width + imageX];
                }
            }

            result.blocks.push_back(block);
        }
    }

    return result;
}

ImageBlocks BlockSplitter::splitImageIntoBlocks(const YCbCrImage& image) {
    if (image.width <= 0 || image.height <= 0) {
        throw std::runtime_error("Invalid image dimensions for block splitting");
    }

    ImageBlocks result;
    result.yBlocks = splitChannelIntoBlocks(image.y, image.width, image.height);
    result.cbBlocks = splitChannelIntoBlocks(image.cb, image.width, image.height);
    result.crBlocks = splitChannelIntoBlocks(image.cr, image.width, image.height);

    return result;
}