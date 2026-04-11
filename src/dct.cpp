#include "dct.hpp"

#include <cmath>
#include <stdexcept>

namespace {
constexpr double kPi = 3.14159265358979323846;

double alpha(int value) {
    return (value == 0) ? (1.0 / std::sqrt(2.0)) : 1.0;
}
}

DctBlock8x8 DCT::forwardDCT(const Block8x8& inputBlock) {
    DctBlock8x8 outputBlock{};

    for (int v = 0; v < 8; ++v) {
        for (int u = 0; u < 8; ++u) {
            double sum = 0.0;

            for (int y = 0; y < 8; ++y) {
                for (int x = 0; x < 8; ++x) {
                    const double shiftedSample =
                        static_cast<double>(inputBlock[y * 8 + x]) - 128.0;

                    const double cosX =
                        std::cos(((2.0 * x + 1.0) * u * kPi) / 16.0);
                    const double cosY =
                        std::cos(((2.0 * y + 1.0) * v * kPi) / 16.0);

                    sum += shiftedSample * cosX * cosY;
                }
            }

            outputBlock[v * 8 + u] =
                0.25 * alpha(u) * alpha(v) * sum;
        }
    }

    return outputBlock;
}

DctChannelBlocks DCT::applyToChannel(const ChannelBlocks& channelBlocks) {
    DctChannelBlocks result;
    result.blocksPerRow = channelBlocks.blocksPerRow;
    result.blocksPerCol = channelBlocks.blocksPerCol;
    result.blocks.reserve(channelBlocks.blocks.size());

    for (const auto& block : channelBlocks.blocks) {
        result.blocks.push_back(forwardDCT(block));
    }

    return result;
}

DctImageBlocks DCT::applyToImage(const ImageBlocks& imageBlocks) {
    DctImageBlocks result;
    result.yBlocks = applyToChannel(imageBlocks.yBlocks);
    result.cbBlocks = applyToChannel(imageBlocks.cbBlocks);
    result.crBlocks = applyToChannel(imageBlocks.crBlocks);
    return result;
}