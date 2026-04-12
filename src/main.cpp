#include "bmp_reader.hpp"
#include "color_converter.hpp"
#include "padding.hpp"
#include "block_splitter.hpp"
#include "dct.hpp"
#include "quantizer.hpp"
#include "zigzag.hpp"
#include "entropy_encoder.hpp"

#include <iostream>

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

        EntropyImageData entropy = EntropyEncoder::encodeImage(zigzagged);

        if (!entropy.y.blocks.empty()) {
            const auto& firstBlock = entropy.y.blocks[0];

            std::cout << "Entropy encoding module test\n";
            std::cout << "First Y block DC difference: " << firstBlock.dc.difference << "\n";
            std::cout << "First Y block DC category: " << firstBlock.dc.category << "\n";
            std::cout << "First Y block DC bits: ";
            printBits(firstBlock.dc.amplitudeBits);
            std::cout << "\n";

            std::cout << "First Y block AC symbol count: " << firstBlock.acValues.size() << "\n";

            for (std::size_t i = 0; i < firstBlock.acValues.size() && i < 5; ++i) {
                const auto& ac = firstBlock.acValues[i];

                std::cout << "AC[" << i << "] ";

                if (ac.isEob) {
                    std::cout << "EOB\n";
                } else if (ac.isZrl) {
                    std::cout << "ZRL\n";
                } else {
                    std::cout << "run=" << ac.runLength
                              << " value=" << ac.value
                              << " size=" << ac.size
                              << " bits=";
                    printBits(ac.amplitudeBits);
                    std::cout << "\n";
                }
            }
        }

        std::cout << "Entropy module verification succeeded.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}