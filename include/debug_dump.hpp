#pragma once

#include "dct.hpp"
#include "quantizer.hpp"
#include "zigzag.hpp"

class DebugDump {
public:
    static void dumpFirstBlock(const DctImageBlocks& dctBlocks,
                               const QuantizedImageBlocks& quantizedBlocks,
                               const ZigZagImageBlocks& zigzagBlocks,
                               bool enabled);
};