#include "bmp_reader.hpp"
#include "color_converter.hpp"
#include "padding.hpp"
#include "block_splitter.hpp"
#include "dct.hpp"
#include "quantizer.hpp"
#include "zigzag.hpp"
#include "dc_encoder.hpp"
#include "ac_encoder.hpp"

#include <iostream>
#include <iomanip>

void printBits(const std::vector<bool>& bits) {
    for (bool bit : bits) {
        std::cout << (bit ? '1' : '0');
    }
}

int main() {
    try {
        BmpImage image = BmpReader::load("../images/input/test.bmp");
        YCbCrImage ycbcr = ColorConverter::rgbToYCbCr(image.data, image.width, image.height);
        YCbCrImage padded = Padding::padToMultipleOf8(ycbcr);
        ImageBlocks blocks = BlockSplitter::splitImageIntoBlocks(padded);
        DctImageBlocks dctBlocks = DCT::applyToImage(blocks);
        QuantizedImageBlocks quantized = Quantizer::quantizeImage(dctBlocks, 50);
        ZigZagImageBlocks zigzagged = ZigZag::reorderImage(quantized);
        AcEncodedImage acEncoded = ACEncoder::encodeImage(zigzagged);

        if (!acEncoded.y.blocks.empty()) {
            std::cout << "First Y AC encoded block:\n";

            const auto& block = acEncoded.y.blocks[0];
            for (std::size_t i = 0; i < block.symbols.size(); ++i) {
                const auto& s = block.symbols[i];

                std::cout << "Symbol " << i << " | ";

                if (s.isEob) {
                    std::cout << "EOB\n";
                } else if (s.isZrl) {
                    std::cout << "ZRL\n";
                } else {
                    std::cout << "run = " << s.runLength
                              << " | value = " << s.value
                              << " | size = " << s.size
                              << " | bits = ";
                    printBits(ACEncoder::amplitudeBits(s.value));
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