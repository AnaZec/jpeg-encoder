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

#include <algorithm>
#include <chrono>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static int parseQuality(int argc, char* argv[]) {
    int quality = 75; // default

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

    return quality;
}

static std::string extractImageNumber(const fs::path& path, int fallbackIndex) {
    const std::string stem = path.stem().string();
    std::string digits;

    for (char c : stem) {
        if (std::isdigit(static_cast<unsigned char>(c))) {
            digits += c;
        }
    }

    if (!digits.empty()) {
        return digits;
    }

    return std::to_string(fallbackIndex);
}

int main(int argc, char* argv[]) {
    try {
        const int quality = parseQuality(argc, argv);

        const fs::path inputDir = "../images/input";
        const fs::path outputDir = "../images/output";
        const fs::path reportDir = "../reports";

        fs::create_directories(outputDir);
        fs::create_directories(reportDir);

        if (!fs::exists(inputDir) || !fs::is_directory(inputDir)) {
            throw std::runtime_error("Input directory does not exist: " + inputDir.string());
        }

        std::vector<fs::path> bmpFiles;
        for (const auto& entry : fs::directory_iterator(inputDir)) {
            if (!entry.is_regular_file()) {
                continue;
            }

            const std::string ext = entry.path().extension().string();
            if (ext == ".bmp" || ext == ".BMP") {
                bmpFiles.push_back(entry.path());
            }
        }

        if (bmpFiles.empty()) {
            throw std::runtime_error("No BMP files found in: " + inputDir.string());
        }

        std::sort(bmpFiles.begin(), bmpFiles.end());

        std::cout << "Found " << bmpFiles.size() << " BMP image(s) in "
                  << inputDir.string() << '\n';
        std::cout << "Using quality = " << quality << "\n\n";

        int processedCount = 0;
        int failedCount = 0;

        for (std::size_t i = 0; i < bmpFiles.size(); ++i) {
            const fs::path& inputPath = bmpFiles[i];
            const std::string imageNumber = extractImageNumber(inputPath, static_cast<int>(i + 1));

            const fs::path outputPath =
                outputDir / ("output" + imageNumber + "_q" + std::to_string(quality) + ".jpg");

            const fs::path reportPath =
                reportDir / ("report" + imageNumber + "_q" + std::to_string(quality) + ".txt");

            try {
                std::ofstream reportFile(reportPath);
                if (!reportFile) {
                    throw std::runtime_error("Failed to open report file: " + reportPath.string());
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

                logSeparator();
                log("Processing image: " + inputPath.filename().string() + "\n");
                log("Image number: " + imageNumber + "\n");
                logValue("Quality: ", quality);
                logSeparator();

                auto cycleStart = std::chrono::high_resolution_clock::now();

                // 1. Load BMP
                auto start = std::chrono::high_resolution_clock::now();
                log("Loading BMP...\n");
                BmpImage bmp = BmpReader::load(inputPath.string());
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
                    outputPath.string(),
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
                log("Input:   " + inputPath.string() + "\n");
                log("Output:  " + outputPath.string() + "\n");
                log("Report:  " + reportPath.string() + "\n");
                logValue("Size:    ", bytesWritten, " bytes");
                logSeparator();

                // 11. Reload images with OpenCV for metric calculation
                cv::Mat original = cv::imread(inputPath.string(), cv::IMREAD_COLOR);
                cv::Mat compressed = cv::imread(outputPath.string(), cv::IMREAD_COLOR);

                if (original.empty()) {
                    throw std::runtime_error("Failed to load original image with OpenCV: " + inputPath.string());
                }

                if (compressed.empty()) {
                    throw std::runtime_error("Failed to load compressed JPEG with OpenCV: " + outputPath.string());
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

                const double totalRuntimeSec =
                    static_cast<double>(totalRuntimeMs.count()) / 1000.0;

                logSeparator();
                {
                    std::ostringstream oss;
                    oss << std::fixed << std::setprecision(3)
                        << "Total runtime: " << totalRuntimeSec << " s\n";
                    log(oss.str());
                }

                log("\nDONE\n\n");
                reportFile.flush();
                ++processedCount;
            } catch (const std::exception& e) {
                std::cerr << "Error while processing " << inputPath.filename().string()
                          << ": " << e.what() << '\n';
                ++failedCount;
            }
        }

        std::cout << "========================================\n";
        std::cout << "Batch processing finished.\n";
        std::cout << "Processed successfully: " << processedCount << '\n';
        std::cout << "Failed: " << failedCount << '\n';
        std::cout << "========================================\n";

        if (processedCount == 0) {
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}