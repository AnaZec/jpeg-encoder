#include "bmp_reader.hpp"
#include "color_converter.hpp"
#include "padding.hpp"
#include "block_splitter.hpp"
#include "dct.hpp"
#include "quantizer.hpp"
#include "zigzag.hpp"
#include "entropy_encoder.hpp"
#include "jpeg_writer.hpp"
#include "metrics.hpp"

#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>

int main() {
    try {
        const std::string inputPath = "../images/input/test.bmp";
        const std::string outputPath = "../images/output/output.jpg";
        const int quality = 75;

        // 1. Load BMP
        BmpImage bmp = BmpReader::load(inputPath);

        // 2. Convert RGB -> YCbCr
        YCbCrImage ycbcr = ColorConverter::rgbToYCbCr(bmp.data, bmp.width, bmp.height);

        // 3. Pad image so dimensions are multiples of 8
        YCbCrImage padded = Padding::padToMultipleOf8(ycbcr);

        // 4. Split into 8x8 blocks
        ImageBlocks blocks = BlockSplitter::splitImageIntoBlocks(padded);

        // 5. Apply DCT
        DctImageBlocks dctBlocks = DCT::applyToImage(blocks);

        // 6. Quantize
        QuantizedImageBlocks quantized = Quantizer::quantizeImage(dctBlocks, quality);

        // 7. Zigzag reorder
        ZigZagImageBlocks zigzag = ZigZag::reorderImage(quantized);

        // 8. Entropy encode
        EntropyImageData entropy = EntropyEncoder::encodeImage(zigzag);

        // 9. Get quantization tables used for this quality factor
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

        std::cout << "JPEG written successfully\n";
        std::cout << "Input:   " << inputPath << "\n";
        std::cout << "Output:  " << outputPath << "\n";
        std::cout << "Quality: " << quality << "\n";
        std::cout << "Size:    " << bytesWritten << " bytes\n";

        // 11. Reload images with OpenCV for metric calculation
        cv::Mat original = cv::imread(inputPath, cv::IMREAD_COLOR);
        cv::Mat compressed = cv::imread(outputPath, cv::IMREAD_COLOR);

        if (original.empty()) {
            throw std::runtime_error("Failed to load original image with OpenCV: " + inputPath);
        }

        if (compressed.empty()) {
            throw std::runtime_error("Failed to load compressed JPEG with OpenCV: " + outputPath);
        }

        // 12. Compute MSE
        MSE mse = Metrics::computeMSE(original, compressed);

        std::cout << "\nMSE:\n";
        std::cout << "  B: " << mse.mseB << "\n";
        std::cout << "  G: " << mse.mseG << "\n";
        std::cout << "  R: " << mse.mseR << "\n";

        // 13. Compute PSNR
        PSNR psnr = Metrics::computePSNR(mse);

        std::cout << "\nPSNR:\n";
        std::cout << "  B: " << psnr.psnrB << " dB\n";
        std::cout << "  G: " << psnr.psnrG << " dB\n";
        std::cout << "  R: " << psnr.psnrR << " dB\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}