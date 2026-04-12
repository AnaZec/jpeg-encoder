#include "entropy_encoder.hpp"

EntropyEncodedBlock EntropyEncoder::encodeBlock(const ZigZagBlock& block, int previousDC) {
    EntropyEncodedBlock result;

    const int currentDC = block[0];
    const int diff = currentDC - previousDC;

    result.dc.difference = diff;
    result.dc.category = DCEncoder::magnitudeCategory(diff);
    result.dc.amplitudeBits = DCEncoder::amplitudeBits(diff);

    const AcEncodedBlock acBlock = ACEncoder::encodeBlock(block);

    result.acValues.reserve(acBlock.symbols.size());

    for (const auto& symbol : acBlock.symbols) {
        EncodedACValue encoded;
        encoded.runLength = symbol.runLength;
        encoded.value = symbol.value;
        encoded.size = symbol.size;
        encoded.isEob = symbol.isEob;
        encoded.isZrl = symbol.isZrl;

        if (!symbol.isEob && !symbol.isZrl) {
            encoded.amplitudeBits = ACEncoder::amplitudeBits(symbol.value);
        }

        result.acValues.push_back(encoded);
    }

    return result;
}

EntropyEncodedChannel EntropyEncoder::encodeChannel(const ZigZagChannelBlocks& channelBlocks) {
    EntropyEncodedChannel result;
    result.blocks.reserve(channelBlocks.blocks.size());

    int previousDC = 0;

    for (const auto& block : channelBlocks.blocks) {
        result.blocks.push_back(encodeBlock(block, previousDC));
        previousDC = block[0];
    }

    return result;
}

EntropyImageData EntropyEncoder::encodeImage(const ZigZagImageBlocks& imageBlocks) {
    EntropyImageData result;
    result.y = encodeChannel(imageBlocks.yBlocks);
    result.cb = encodeChannel(imageBlocks.cbBlocks);
    result.cr = encodeChannel(imageBlocks.crBlocks);
    return result;
}