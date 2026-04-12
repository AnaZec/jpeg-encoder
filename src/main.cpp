#include "bmp_reader.hpp"
#include "color_converter.hpp"
#include "padding.hpp"
#include "block_splitter.hpp"
#include "dct.hpp"
#include "quantizer.hpp"
#include "zigzag.hpp"
#include "debug_dump.hpp"

#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    try {
        bool debugMode = false;

        for (int i = 1; i < argc; ++i) {
            if (std::string(argv[i]) == "--debug") {
                debugMode = true;
            }
        }

        BmpImage image = BmpReader::load("../images/input/test.bmp");
        YCbCrImage ycbcr = ColorConverter::rgbToYCbCr(image.data, image.width, image.height);
        YCbCrImage padded = Padding::padToMultipleOf8(ycbcr);
        ImageBlocks blocks = BlockSplitter::splitImageIntoBlocks(padded);
        DctImageBlocks dctBlocks = DCT::applyToImage(blocks);
        QuantizedImageBlocks quantizedBlocks = Quantizer::quantizeImage(dctBlocks, 50);
        ZigZagImageBlocks zigzagBlocks = ZigZag::reorderImage(quantizedBlocks);

        DebugDump::dumpFirstBlock(dctBlocks, quantizedBlocks, zigzagBlocks, debugMode);

        std::cout << "Pipeline completed successfully.\n";
        std::cout << "Debug mode: " << (debugMode ? "ON" : "OFF") << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}