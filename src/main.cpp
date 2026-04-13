#include "bmp_reader.hpp"
#include "color_converter.hpp"
#include "padding.hpp"
#include "block_splitter.hpp"
#include "dct.hpp"
#include "quantizer.hpp"
#include "zigzag.hpp"
#include "entropy_encoder.hpp"
#include "jpeg_writer.hpp"

#include <iostream>
#include <string>

int main() {
    try {
        const std::string inputPath = "../images/input/test.bmp";
        const std::string outputPath = "../images/output/output.jpg";

        // 1. Load BMP
        BmpImage bmp = BmpReader::load(inputPath);

        // 2. RGB -> YCbCr
        YCbCrImage ycbcr = ColorConverter::rgbToYCbCr(bmp.data, bmp.width, bmp.height);

        // 3. Padding
        YCbCrImage padded = Padding::padToMultipleOf8(ycbcr);

        // 4. Split into 8x8 blocks
        ImageBlocks blocks = BlockSplitter::splitImageIntoBlocks(padded);

        // 5. DCT
        DctImageBlocks dctBlocks = DCT::applyToImage(blocks);

        // 6. Quantization
        const int quality = 75;
        QuantizedImageBlocks quantized = Quantizer::quantizeImage(dctBlocks, quality);

        // 7. Zigzag
        ZigZagImageBlocks zigzag = ZigZag::reorderImage(quantized);

        // 8. Entropy encoding
        EntropyImageData entropy = EntropyEncoder::encodeImage(zigzag);

        // 9. Quantization tables
        const auto lumaTable = Quantizer::scaledLuminanceTable(quality);
        const auto chromaTable = Quantizer::scaledChrominanceTable(quality);

        // 10. Write JPEG
        std::size_t bytesWritten = JpegWriter::writeJpegFile(
            outputPath,
            bmp.width,
            bmp.height,
            lumaTable,
            chromaTable,
            entropy
        );

        std::cout << "JPEG written successfully!\n";
        std::cout << "Output: " << outputPath << "\n";
        std::cout << "Size: " << bytesWritten << " bytes\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
