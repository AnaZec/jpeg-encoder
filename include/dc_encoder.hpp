#pragma once

#include "zigzag.hpp"
#include <vector>

struct DcDifferenceChannel {
    std::vector<int> differences;
};

struct DcDifferenceImage {
    DcDifferenceChannel y;
    DcDifferenceChannel cb;
    DcDifferenceChannel cr;
};

class DCEncoder {
public:
    static DcDifferenceChannel computeDifferences(const ZigZagChannelBlocks& channelBlocks);
    static DcDifferenceImage computeImageDifferences(const ZigZagImageBlocks& imageBlocks);

    static int magnitudeCategory(int value);
    static std::vector<bool> amplitudeBits(int value);
};