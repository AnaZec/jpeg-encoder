#pragma once

#include "zigzag.hpp"
#include "dc_encoder.hpp"
#include "ac_encoder.hpp"
#include "huffman_tables.hpp"

#include <vector>

struct EncodedDCValue {
    int difference = 0;
    int category = 0;
    std::vector<bool> amplitudeBits;
};

struct EncodedACValue {
    int runLength = 0;
    int value = 0;
    int size = 0;
    std::vector<bool> amplitudeBits;
    bool isEob = false;
    bool isZrl = false;
};

struct EntropyEncodedBlock {
    EncodedDCValue dc;
    std::vector<EncodedACValue> acValues;
};

struct EntropyEncodedChannel {
    std::vector<EntropyEncodedBlock> blocks;
};

struct EntropyImageData {
    EntropyEncodedChannel y;
    EntropyEncodedChannel cb;
    EntropyEncodedChannel cr;
};

class EntropyEncoder {
public:
    static EntropyEncodedBlock encodeBlock(const ZigZagBlock& block, int previousDC);
    static EntropyEncodedChannel encodeChannel(const ZigZagChannelBlocks& channelBlocks);
    static EntropyImageData encodeImage(const ZigZagImageBlocks& imageBlocks);
};