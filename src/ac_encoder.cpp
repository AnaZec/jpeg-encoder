#include "ac_encoder.hpp"

#include <cmath>
#include <stdexcept>

AcEncodedBlock ACEncoder::encodeBlock(const ZigZagBlock& block) {
    AcEncodedBlock result;

    int zeroRun = 0;

    // index 0 is DC
    for (int i = 1; i < 64; ++i) {
        const int value = block[i];

        if (value == 0) {
            ++zeroRun;

            // JPEG uses ZRL for each full run of 16 zeros
            if (zeroRun == 16) {
                AcSymbol zrl;
                zrl.runLength = 15;
                zrl.value = 0;
                zrl.size = 0;
                zrl.isZrl = true;
                result.symbols.push_back(zrl);
                zeroRun = 0;
            }

            continue;
        }

        AcSymbol symbol;
        symbol.runLength = zeroRun;
        symbol.value = value;
        symbol.size = magnitudeCategory(value);
        result.symbols.push_back(symbol);

        zeroRun = 0;
    }

    // If trailing coefficients are zero -- append EOB
    if (zeroRun > 0) {
        AcSymbol eob;
        eob.runLength = 0;
        eob.value = 0;
        eob.size = 0;
        eob.isEob = true;
        result.symbols.push_back(eob);
    }

    return result;
}

AcEncodedChannel ACEncoder::encodeChannel(const ZigZagChannelBlocks& channelBlocks) {
    AcEncodedChannel result;
    result.blocks.reserve(channelBlocks.blocks.size());

    for (const auto& block : channelBlocks.blocks) {
        result.blocks.push_back(encodeBlock(block));
    }

    return result;
}

AcEncodedImage ACEncoder::encodeImage(const ZigZagImageBlocks& imageBlocks) {
    AcEncodedImage result;
    result.y = encodeChannel(imageBlocks.yBlocks);
    result.cb = encodeChannel(imageBlocks.cbBlocks);
    result.cr = encodeChannel(imageBlocks.crBlocks);
    return result;
}

int ACEncoder::magnitudeCategory(int value) {
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

std::vector<bool> ACEncoder::amplitudeBits(int value) {
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