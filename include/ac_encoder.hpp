#pragma once

#include "zigzag.hpp"
#include <vector>

struct AcSymbol {
    int runLength = 0;   // number of zeros before the value
    int value = 0;       // nonzero AC coefficient
    int size = 0;        // magnitude category of value
    bool isEob = false;  // End of Block marker
    bool isZrl = false;  // Zero Run Length marker (16 zeros)
};

struct AcEncodedBlock {
    std::vector<AcSymbol> symbols;
};

struct AcEncodedChannel {
    std::vector<AcEncodedBlock> blocks;
};

struct AcEncodedImage {
    AcEncodedChannel y;
    AcEncodedChannel cb;
    AcEncodedChannel cr;
};

class ACEncoder {
public:
    static AcEncodedBlock encodeBlock(const ZigZagBlock& block);
    static AcEncodedChannel encodeChannel(const ZigZagChannelBlocks& channelBlocks);
    static AcEncodedImage encodeImage(const ZigZagImageBlocks& imageBlocks);

    static int magnitudeCategory(int value);
    static std::vector<bool> amplitudeBits(int value);
};