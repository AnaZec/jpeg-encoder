#include "debug_dump.hpp"

#include <iomanip>
#include <iostream>

namespace {

void printDctBlock(const DctBlock8x8& block) {
    std::cout << "DCT coefficients (first Y block):\n";
    for (int i = 0; i < 64; ++i) {
        std::cout << std::setw(10) << std::fixed << std::setprecision(2) << block[i] << " ";
        if ((i + 1) % 8 == 0) {
            std::cout << "\n";
        }
    }
    std::cout << "\n";
}

void printQuantizedBlock(const QuantizedBlock8x8& block) {
    std::cout << "Quantized coefficients (first Y block):\n";
    for (int i = 0; i < 64; ++i) {
        std::cout << std::setw(5) << block[i] << " ";
        if ((i + 1) % 8 == 0) {
            std::cout << "\n";
        }
    }
    std::cout << "\n";
}

void printZigZagBlock(const ZigZagBlock& block) {
    std::cout << "Zigzag coefficients (first Y block):\n";
    for (int i = 0; i < 64; ++i) {
        std::cout << std::setw(5) << block[i] << " ";
        if ((i + 1) % 8 == 0) {
            std::cout << "\n";
        }
    }
    std::cout << "\n";
}

} // namespace

void DebugDump::dumpFirstBlock(const DctImageBlocks& dctBlocks,
                               const QuantizedImageBlocks& quantizedBlocks,
                               const ZigZagImageBlocks& zigzagBlocks,
                               bool enabled) {
    if (!enabled) {
        return;
    }

    if (dctBlocks.yBlocks.blocks.empty() ||
        quantizedBlocks.yBlocks.blocks.empty() ||
        zigzagBlocks.yBlocks.blocks.empty()) {
        std::cout << "Debug dump skipped: no Y blocks available.\n";
        return;
    }

    std::cout << "========== DEBUG DUMP ==========\n\n";

    printDctBlock(dctBlocks.yBlocks.blocks[0]);
    printQuantizedBlock(quantizedBlocks.yBlocks.blocks[0]);
    printZigZagBlock(zigzagBlocks.yBlocks.blocks[0]);

    std::cout << "======== END DEBUG DUMP ========\n";
}