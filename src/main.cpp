#include "bmp_reader.hpp"
#include "color_converter.hpp"
#include "padding.hpp"
#include "block_splitter.hpp"
#include "dct.hpp"
#include "quantizer.hpp"
#include "zigzag.hpp"

#include <iostream>
#include <iomanip>

int main() {
    try {
        BmpImage image = BmpReader::load("../images/input/test.bmp");
        YCbCrImage ycbcr = ColorConverter::rgbToYCbCr(image.data, image.width, image.height);
        YCbCrImage padded = Padding::padToMultipleOf8(ycbcr);
        ImageBlocks blocks = BlockSplitter::splitImageIntoBlocks(padded);
        DctImageBlocks dctBlocks = DCT::applyToImage(blocks);
        QuantizedImageBlocks quantized = Quantizer::quantizeImage(dctBlocks, 50);
        ZigZagImageBlocks zigzagged = ZigZag::reorderImage(quantized);

        if (!quantized.yBlocks.blocks.empty() && !zigzagged.yBlocks.blocks.empty()) {
            std::cout << "First quantized Y block (row-major):\n";
            for (int i = 0; i < 64; ++i) {
                std::cout << std::setw(4) << quantized.yBlocks.blocks[0][i] << " ";
                if ((i + 1) % 8 == 0) {
                    std::cout << "\n";
                }
            }

            std::cout << "\nFirst zigzagged Y block:\n";
            for (int i = 0; i < 64; ++i) {
                std::cout << std::setw(4) << zigzagged.yBlocks.blocks[0][i] << " ";
                if ((i + 1) % 8 == 0) {
                    std::cout << "\n";
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}