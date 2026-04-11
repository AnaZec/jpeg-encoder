#include "bmp_reader.hpp"
#include "color_converter.hpp"
#include "padding.hpp"
#include "block_splitter.hpp"
#include "dct.hpp"

#include <iostream>
#include <iomanip>

int main() {
    try {
        BmpImage image = BmpReader::load("../images/input/test.bmp");
        YCbCrImage ycbcr = ColorConverter::rgbToYCbCr(image.data, image.width, image.height);
        YCbCrImage padded = Padding::padToMultipleOf8(ycbcr);
        ImageBlocks blocks = BlockSplitter::splitImageIntoBlocks(padded);
        DctImageBlocks dctBlocks = DCT::applyToImage(blocks);

        std::cout << "Padded size: " << padded.width << " x " << padded.height << "\n";
        std::cout << "Total Y blocks: " << dctBlocks.yBlocks.blocks.size() << "\n";

        if (!dctBlocks.yBlocks.blocks.empty()) {
            std::cout << "First Y DCT block:\n";
            const auto& block = dctBlocks.yBlocks.blocks[0];

            for (int i = 0; i < 64; ++i) {
                std::cout << std::setw(10) << std::fixed << std::setprecision(2)
                          << block[i] << " ";
                if ((i + 1) % 8 == 0) {
                    std::cout << "\n";
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    // Quick check DCT
    Block8x8 constantBlock{};
    constantBlock.fill(128);

    DctBlock8x8 dct = DCT::forwardDCT(constantBlock);

    std::cout << "DCT of constant 128 block:\n";
    for (int i = 0; i < 64; ++i) {
        std::cout << dct[i] << " ";
        if ((i + 1) % 8 == 0) {
            std::cout << "\n";
        }
    }

    return 0;
}