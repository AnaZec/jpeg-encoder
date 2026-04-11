#include "bmp_reader.hpp"
#include "color_converter.hpp"
#include "padding.hpp"
#include "block_splitter.hpp"
#include "dct.hpp"
#include "quantizer.hpp"

#include <iostream>
#include <iomanip>

int main() {
    try {
        BmpImage image = BmpReader::load("../images/input/test.bmp");
        YCbCrImage ycbcr = ColorConverter::rgbToYCbCr(image.data, image.width, image.height);
        YCbCrImage padded = Padding::padToMultipleOf8(ycbcr);
        ImageBlocks blocks = BlockSplitter::splitImageIntoBlocks(padded);
        DctImageBlocks dctBlocks = DCT::applyToImage(blocks);

        const int qualityFactor = 50;
        QuantizedImageBlocks quantized = Quantizer::quantizeImage(dctBlocks, qualityFactor);

        std::cout << "Quality factor: " << qualityFactor << "\n";
        std::cout << "Total Y blocks: " << quantized.yBlocks.blocks.size() << "\n";

        if (!quantized.yBlocks.blocks.empty()) {
            std::cout << "First quantized Y block:\n";
            const auto& block = quantized.yBlocks.blocks[0];

            for (int i = 0; i < 64; ++i) {
                std::cout << std::setw(5) << block[i] << " ";
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