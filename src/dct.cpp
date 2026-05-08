#include "dct.hpp"

#include <array>
#include <cmath>
#include <stdexcept>

namespace {

constexpr int kBlockSize = 8;
constexpr int kBlockArea = kBlockSize * kBlockSize;
constexpr double kPi = 3.14159265358979323846;

using CosineTable = std::array<std::array<double, kBlockSize>, kBlockSize>;
using ScaleTable = std::array<std::array<double, kBlockSize>, kBlockSize>;

double alpha(int value) {
    return (value == 0) ? (1.0 / std::sqrt(2.0)) : 1.0;
}

const CosineTable& cosineTable() {
    static const CosineTable table = [] {
        CosineTable values{};

        for (int frequency = 0; frequency < kBlockSize; ++frequency) {
            for (int position = 0; position < kBlockSize; ++position) {
                values[frequency][position] =
                    std::cos(((2.0 * position + 1.0) *
                              frequency *
                              kPi) / 16.0);
            }
        }

        return values;
    }();

    return table;
}

const ScaleTable& dctScaleTable() {
    static const ScaleTable table = [] {
        ScaleTable values{};

        for (int v = 0; v < kBlockSize; ++v) {
            for (int u = 0; u < kBlockSize; ++u) {
                values[v][u] = 0.25 * alpha(u) * alpha(v);
            }
        }

        return values;
    }();

    return table;
}

} // namespace

DctBlock8x8 DCT::forwardDCT(const Block8x8& inputBlock) {
    DctBlock8x8 outputBlock{};

    const auto& cosines = cosineTable();
    const auto& scales = dctScaleTable();

    for (int v = 0; v < kBlockSize; ++v) {
        for (int u = 0; u < kBlockSize; ++u) {
            double sum = 0.0;

            for (int y = 0; y < kBlockSize; ++y) {
                const int rowOffset = y * kBlockSize;
                const double cosY = cosines[v][y];

                for (int x = 0; x < kBlockSize; ++x) {
                    const double shiftedSample =
                        static_cast<double>(inputBlock[rowOffset + x]) - 128.0;

                    sum += shiftedSample * cosines[u][x] * cosY;
                }
            }

            outputBlock[v * kBlockSize + u] = scales[v][u] * sum;
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