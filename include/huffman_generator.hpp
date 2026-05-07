#pragma once

#include "entropy_encoder.hpp"
#include "huffman_tables.hpp"

#include <array>
#include <cstdint>

class HuffmanGenerator {
public:
    using FrequencyTable = std::array<uint32_t, 256>;

    static JpegHuffmanTableSet generateFromEntropyData(const EntropyImageData& entropyData);

private:
    static JpegHuffmanTable generateFromFrequencies(const FrequencyTable& frequencies);

    static void countChannelFrequencies(const EntropyEncodedChannel& channel,
                                        FrequencyTable& dcFrequencies,
                                        FrequencyTable& acFrequencies);

    static uint8_t makeAcSymbol(const EncodedACValue& value);
};