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

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char* argv[]) {
    try {
        int quality = 75; // default quality

        // Parse CLI args: ./jpeg_encoder --quality 75
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];

            if (arg == "--quality") {
                if (i + 1 >= argc) {
                    throw std::runtime_error("Missing value for --quality");
                }
                quality = std::stoi(argv[++i]);
            } else {
                throw std::runtime_error("Unknown argument: " + arg);
            }
        }

        if (quality < 1 || quality > 100) {
            throw std::runtime_error("Quality must be in range [1, 100]");
        }

        const std::string inputPath = "../images/input/test.bmp";
        const std::string outputDir = "../images/output";
        const std::string reportDir = "../reports";

        std::filesystem::create_directories(outputDir);
        std::filesystem::create_directories(reportDir);

        const std::string outputPath =
            outputDir + "/output_q" + std::to_string(quality) + ".jpg";

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

        auto logValue = [&](const std::string& label, auto value, const std::string& suffix = "") {
            std::ostringstream oss;
            oss << label << value << suffix << '\n';
            std::cout << oss.str();
            reportFile << oss.str();
        };

        auto logSeparator = [&]() {
            log("**************************************************\n");
        };

        auto logPhaseDuration = [&](const std::chrono::milliseconds& duration) {
            logValue("Phase duration: ", duration.count(), " ms");
            logSeparator();
        };

        auto cycleStart = std::chrono::high_resolution_clock::now();

        // 1. Load BMP
        auto start = std::chrono::high_resolution_clock::now();
        log("Loading BMP...\n");
        BmpImage bmp = BmpReader::load(inputPath);
        auto end = std::chrono::high_resolution_clock::now();
        logPhaseDuration(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));

        // 2. Convert RGB -> YCbCr
        start = std::chrono::high_resolution_clock::now();
        log("Converting RGB to YCbCr...\n");
        YCbCrImage ycbcr = ColorConverter::rgbToYCbCr(bmp.data, bmp.width, bmp.height);
        end = std::chrono::high_resolution_clock::now();
        logPhaseDuration(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));

        // 3. Pad image so dimensions are multiples of 8
        start = std::chrono::high_resolution_clock::now();
        log("Padding image to dimensions multiple of 8...\n");
        YCbCrImage padded = Padding::padToMultipleOf8(ycbcr);
        end = std::chrono::high_resolution_clock::now();
        logPhaseDuration(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));

        // 4. Split into 8x8 blocks
        start = std::chrono::high_resolution_clock::now();
        log("Splitting into 8x8 blocks...\n");
        ImageBlocks blocks = BlockSplitter::splitImageIntoBlocks(padded);
        end = std::chrono::high_resolution_clock::now();
        logPhaseDuration(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));

        // 5. Apply DCT
        start = std::chrono::high_resolution_clock::now();
        log("Applying DCT...\n");
        DctImageBlocks dctBlocks = DCT::applyToImage(blocks);
        end = std::chrono::high_resolution_clock::now();
        logPhaseDuration(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));

        // 6. Quantize
        start = std::chrono::high_resolution_clock::now();
        log("Quantizing...\n");
        QuantizedImageBlocks quantized = Quantizer::quantizeImage(dctBlocks, quality);
        end = std::chrono::high_resolution_clock::now();
        logPhaseDuration(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));

        // 7. Zigzag reorder
        start = std::chrono::high_resolution_clock::now();
        log("Zigzag reorder...\n");
        ZigZagImageBlocks zigzag = ZigZag::reorderImage(quantized);
        end = std::chrono::high_resolution_clock::now();
        logPhaseDuration(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));

        // 8. Entropy encode
        start = std::chrono::high_resolution_clock::now();
        log("Entropy encoding...\n");
        EntropyImageData entropy = EntropyEncoder::encodeImage(zigzag);
        end = std::chrono::high_resolution_clock::now();
        logPhaseDuration(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));

        // 9. Get quantization tables
        start = std::chrono::high_resolution_clock::now();
        log("Getting quantization tables...\n");
        const auto lumaTable = Quantizer::scaledLuminanceTable(quality);
        const auto chromaTable = Quantizer::scaledChrominanceTable(quality);
        end = std::chrono::high_resolution_clock::now();
        logPhaseDuration(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));

        // 10. Write JPEG
        start = std::chrono::high_resolution_clock::now();
        log("Writing JPEG...\n");
        std::size_t bytesWritten = JpegWriter::writeJpegFile(
            outputPath,
            bmp.width,
            bmp.height,
            lumaTable,
            chromaTable,
            entropy
        );
        end = std::chrono::high_resolution_clock::now();
        logPhaseDuration(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));

        auto cycleEnd = std::chrono::high_resolution_clock::now();
        auto totalRuntimeMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(cycleEnd - cycleStart);

        log("JPEG written successfully\n");
        log("Input:   " + inputPath + "\n");
        log("Output:  " + outputPath + "\n");
        log("Report:  " + reportPath + "\n");
        logValue("Quality: ", quality);
        logValue("Size:    ", bytesWritten, " bytes");
        logSeparator();

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

        log("\nMSE:\n");
        logValue("  B: ", mse.mseB);
        logValue("  G: ", mse.mseG);
        logValue("  R: ", mse.mseR);

        // 13. Compute PSNR
        PSNR psnr = Metrics::computePSNR(mse);

        log("\nPSNR:\n");
        logValue("  B: ", psnr.psnrB, " dB");
        logValue("  G: ", psnr.psnrG, " dB");
        logValue("  R: ", psnr.psnrR, " dB");

        const double totalRuntimeSec = static_cast<double>(totalRuntimeMs.count()) / 1000.0;

        logSeparator();
        {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(3)
                << "Total runtime: " << totalRuntimeSec << " s\n";
            log(oss.str());
        }

        reportFile.flush();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}