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
#include <chrono>
#include <fstream>
#include <filesystem>
#include <sstream>

int main(int argc, char* argv[]) {
    try {
        int quality = 75; // default

        // Parse CLI args
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];

            if (arg == "--quality") {
                if (i + 1 >= argc) {
                    throw std::runtime_error("Missing value for --quality");
                }
                quality = std::stoi(argv[++i]);
            }
        }

        if (quality < 1 || quality > 100) {
            throw std::runtime_error("Quality must be in range [1, 100]");
        }

        const std::string inputPath = "../images/input/test.bmp";
        const std::string outputPath = "../images/output/output_q" + std::to_string(quality) + ".jpg";

        // For report generation
        const std::string reportDir = "../reports";
        std::filesystem::create_directories(reportDir);

        const std::string reportPath =
        reportDir + "/report_q" + std::to_string(quality) + ".txt";

        std::ofstream reportFile(reportPath);
        if (!reportFile) {
        throw std::runtime_error("Failed to open report file: " + reportPath);
        }

        auto log = [&](const std::string& message) {
        std::cout << message;
        reportFile << message;
        };

        // Full duration measurement start
        auto start_cycle = std::chrono::high_resolution_clock::now();


        // 1. Load BMP
        auto start = std::chrono::high_resolution_clock::now();
        std::cout << "Loading BMP..." << std::endl;
        BmpImage bmp = BmpReader::load(inputPath);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Phase duration:" << duration_in_ms.count() << " ms" << std::endl;
        std::cout << "**************************************************" << std::endl;


        // 2. Convert RGB -> YCbCr
        start = std::chrono::high_resolution_clock::now();
        std::cout << "Converting RGB to YCbCr..." << std::endl;
        YCbCrImage ycbcr = ColorConverter::rgbToYCbCr(bmp.data, bmp.width, bmp.height);
        end = std::chrono::high_resolution_clock::now();
        duration_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Phase duration:" << duration_in_ms.count() << " ms" << std::endl;
        std::cout << "**************************************************" << std::endl;

        // 3. Pad image so dimensions are multiples of 8
        start = std::chrono::high_resolution_clock::now();
        std::cout << "Padding image to dimensions multiples of 8..." <<std::endl;
        YCbCrImage padded = Padding::padToMultipleOf8(ycbcr);
        end = std::chrono::high_resolution_clock::now();
        duration_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Phase duration:" << duration_in_ms.count() << " ms" << std::endl;
        std::cout << "**************************************************" << std::endl;

        // 4. Split into 8x8 blocks
        start = std::chrono::high_resolution_clock::now();
        std::cout << "Splitting into 8x8 blocks..." << std::endl;
        ImageBlocks blocks = BlockSplitter::splitImageIntoBlocks(padded);
        end = std::chrono::high_resolution_clock::now();
        duration_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Phase duration:" << duration_in_ms.count() << " ms" << std::endl;
        std::cout << "**************************************************" << std::endl;

        // 5. Apply DCT
        start = std::chrono::high_resolution_clock::now();
        std::cout << "Applying DCT..." << std::endl;
        DctImageBlocks dctBlocks = DCT::applyToImage(blocks);
        end = std::chrono::high_resolution_clock::now();
        duration_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Phase duration:" << duration_in_ms.count() << " ms" << std::endl;
        std::cout << "**************************************************" << std::endl;

        // 6. Quantize
        start = std::chrono::high_resolution_clock::now();
        std::cout << "Quantizing..." << std::endl;
        QuantizedImageBlocks quantized = Quantizer::quantizeImage(dctBlocks, quality);
        end = std::chrono::high_resolution_clock::now();
        duration_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Phase duration:" << duration_in_ms.count() << " ms" << std::endl;
        std::cout << "**************************************************" << std::endl;

        // 7. Zigzag reorder
        start = std::chrono::high_resolution_clock::now();
        std::cout << "Zigzag reorder..." << std::endl;
        ZigZagImageBlocks zigzag = ZigZag::reorderImage(quantized);
        end = std::chrono::high_resolution_clock::now();
        duration_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Phase duration:" << duration_in_ms.count() << " ms" << std::endl;
        std::cout << "**************************************************" << std::endl;

        // 8. Entropy encode
        start = std::chrono::high_resolution_clock::now();
        std::cout << "Entropy encoding ..." << std::endl;
        EntropyImageData entropy = EntropyEncoder::encodeImage(zigzag);
        end = std::chrono::high_resolution_clock::now();
        duration_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Phase duration:" << duration_in_ms.count() << " ms" << std::endl;
        std::cout << "**************************************************" << std::endl;

        // 9. Get quantization tables used for this quality factor
        start = std::chrono::high_resolution_clock::now();
        std::cout << "Get quantization tables..." <<std::endl;
        const auto lumaTable = Quantizer::scaledLuminanceTable(quality);
        const auto chromaTable = Quantizer::scaledChrominanceTable(quality);
        end = std::chrono::high_resolution_clock::now();
        duration_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Phase duration:" << duration_in_ms.count() << " ms" << std::endl;
        std::cout << "**************************************************" << std::endl;

        // 10. Write JPEG
        start = std::chrono::high_resolution_clock::now();
        std::cout << "Write JPEG..." << std::endl;
        std::size_t bytesWritten = JpegWriter::writeJpegFile(
            outputPath,
            bmp.width,
            bmp.height,
            lumaTable,
            chromaTable,
            entropy
        );
        end = std::chrono::high_resolution_clock::now();
        duration_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Phase duration:" << duration_in_ms.count() << " ms" << std::endl;
        std::cout << "**************************************************" << std::endl;

        auto end_cycle = std::chrono::high_resolution_clock::now();
        auto duration_in_ms_total = std::chrono::duration_cast<std::chrono::milliseconds>(end_cycle - start_cycle);


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
        std::cout << "  R: " << psnr.psnrR << " dB\n\n";
        std::cout << "**************************************************" << std::endl;

        std::cout << "Total runtime : " << duration_in_ms_total.count()/1000 << " s\n\n" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}