#include "dc_encoder.hpp"

#include <cmath>
#include <stdexcept>

DcDifferenceChannel DCEncoder::computeDifferences(const ZigZagChannelBlocks& channelBlocks) {
    DcDifferenceChannel result;

    int previousDC = 0;
    result.differences.reserve(channelBlocks.blocks.size());

    for (const auto& block : channelBlocks.blocks) {
        const int currentDC = block[0];
        const int diff = currentDC - previousDC;
        result.differences.push_back(diff);
        previousDC = currentDC;
    }

    return result;
}

DcDifferenceImage DCEncoder::computeImageDifferences(const ZigZagImageBlocks& imageBlocks) {
    DcDifferenceImage result;
    result.y = computeDifferences(imageBlocks.yBlocks);
    result.cb = computeDifferences(imageBlocks.cbBlocks);
    result.cr = computeDifferences(imageBlocks.crBlocks);
    return result;
}

int DCEncoder::magnitudeCategory(int value) {
    if (value == 0) {
        return 0;
    }

    int absValue = std::abs(value);
    int category = 0;

    while (absValue > 0) {
        ++category;
        absValue >>= 1;
    }

    return category;
}

std::vector<bool> DCEncoder::amplitudeBits(int value) {
    std::vector<bool> bits;

    const int category = magnitudeCategory(value);
    if (category == 0) {
        return bits;
    }

    if (value > 0) {
        for (int i = category - 1; i >= 0; --i) {
            bits.push_back((value >> i) & 1);
        }
    } else {
        const int mask = (1 << category) - 1;
        const int encodedValue = value + mask;

        for (int i = category - 1; i >= 0; --i) {
            bits.push_back((encodedValue >> i) & 1);
        }
    }

    return bits;
}