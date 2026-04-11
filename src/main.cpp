#include "bmp_reader.hpp"
#include "color_converter.hpp"
#include "padding.hpp"
#include "block_splitter.hpp"
#include "dct.hpp"
#include "quantizer.hpp"
#include "zigzag.hpp"
#include "dc_encoder.hpp"

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
        DcDifferenceImage dcDiffs = DCEncoder::computeImageDifferences(zigzagged);

        std::cout << "First 10 Y DC differences:\n";
        for (std::size_t i = 0; i < 10 && i < dcDiffs.y.differences.size(); ++i) {
            const int diff = dcDiffs.y.differences[i];
            const int category = DCEncoder::magnitudeCategory(diff);
            const auto bits = DCEncoder::amplitudeBits(diff);

            std::cout << "Block " << i
                      << " | diff = " << diff
                      << " | category = " << category
                      << " | bits = ";
            printBits(bits);
            std::cout << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}