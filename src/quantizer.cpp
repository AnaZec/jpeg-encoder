#include "quantizer.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {

// Standard JPEG luminance quantization table used for Y channel
const std::array<int, 64> kLuminanceTable = {
    16, 11, 10, 16, 24, 40, 51, 61,
    12, 12, 14, 19, 26, 58, 60, 55,
    14, 13, 16, 24, 40, 57, 69, 56,
    14, 17, 22, 29, 51, 87, 80, 62,
    18, 22, 37, 56, 68,109,103, 77,
    24, 35, 55, 64, 81,104,113, 92,
    49, 64, 78, 87,103,121,120,101,
    72, 92, 95, 98,112,100,103, 99
};

// Standard JPEG chrominance quantization table used for Cb and Cr channels
const std::array<int, 64> kChrominanceTable = {
    17, 18, 24, 47, 99, 99, 99, 99,
    18, 21, 26, 66, 99, 99, 99, 99,
    24, 26, 56, 99, 99, 99, 99, 99,
    47, 66, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99
};

// Standard JPEG quality scaling formula.
// qualityFactor must be in range [1, 100]
// Lower quality => larger quantization values => more compression.
// Higher quality => smaller quantization values => less compression.
std::array<int, 64> scaleTable(const std::array<int, 64>& baseTable, int qualityFactor) {
    if (qualityFactor < 1 || qualityFactor > 100) {
        throw std::runtime_error("Quality factor must be in range [1, 100]");
    }

    const int scale = (qualityFactor < 50)
        ? (5000 / qualityFactor)
        : (200 - 2 * qualityFactor);

    std::array<int, 64> scaled{};

    for (std::size_t i = 0; i < 64; ++i) {
        int value = (baseTable[i] * scale + 50) / 100;
        value = std::clamp(value, 1, 255);
        scaled[i] = value;
    }

    return scaled;
}

} // namespace

QuantizedBlock8x8 Quantizer::quantizeBlock(const DctBlock8x8& block,
                                           const std::array<int, 64>& table) {
    QuantizedBlock8x8 result{};

    for (std::size_t i = 0; i < 64; ++i) {
        result[i] = static_cast<int>(std::round(block[i] / table[i]));
    }

    return result;
}

QuantizedChannelBlocks Quantizer::quantizeChannel(const DctChannelBlocks& channelBlocks,
                                                  const std::array<int, 64>& table) {
    QuantizedChannelBlocks result;
    result.blocksPerRow = channelBlocks.blocksPerRow;
    result.blocksPerCol = channelBlocks.blocksPerCol;
    result.blocks.reserve(channelBlocks.blocks.size());

    for (const auto& block : channelBlocks.blocks) {
        result.blocks.push_back(quantizeBlock(block, table));
    }

    return result;
}

QuantizedImageBlocks Quantizer::quantizeImage(const DctImageBlocks& imageBlocks,
                                              int qualityFactor) {
    const auto lumTable = scaledLuminanceTable(qualityFactor);
    const auto chromaTable = scaledChrominanceTable(qualityFactor);

    QuantizedImageBlocks result;
    result.yBlocks = quantizeChannel(imageBlocks.yBlocks, lumTable);
    result.cbBlocks = quantizeChannel(imageBlocks.cbBlocks, chromaTable);
    result.crBlocks = quantizeChannel(imageBlocks.crBlocks, chromaTable);

    return result;
}

std::array<int, 64> Quantizer::scaledLuminanceTable(int qualityFactor) {
    return scaleTable(kLuminanceTable, qualityFactor);
}

std::array<int, 64> Quantizer::scaledChrominanceTable(int qualityFactor) {
    return scaleTable(kChrominanceTable, qualityFactor);
}