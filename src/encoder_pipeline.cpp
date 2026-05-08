#include "encoder_pipeline.hpp"

#include "bmp_reader.hpp"
#include "block_splitter.hpp"
#include "color_converter.hpp"
#include "dct.hpp"
#include "entropy_encoder.hpp"
#include "jpeg_writer.hpp"
#include "metrics.hpp"
#include "padding.hpp"
#include "quantizer.hpp"
#include "visual_comparison.hpp"
#include "zigzag.hpp"

#include <opencv2/opencv.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace {

template <typename T>
void releaseMemory(T& object) {
    T empty{};
    object = std::move(empty);
}

} // namespace

EncoderPipeline::EncoderPipeline(EncoderPipelineConfig config)
    : config_(std::move(config)) {}

EncoderPipelineResult EncoderPipeline::processImage(const std::filesystem::path& inputPath,
                                                    const std::string& imageNumber,
                                                    Logger& logger) const {
    EncoderPipelineResult result;
    result.inputPath = inputPath;

    result.outputPath =
        config_.outputDir / ("output" + imageNumber + "_q" +
                             std::to_string(config_.quality) + ".jpg");

    result.reportPath =
        config_.reportDir / ("report" + imageNumber + "_q" +
                             std::to_string(config_.quality) + ".txt");

    result.comparisonPath =
        config_.comparisonDir / ("comparison" + imageNumber + "_q" +
                                 std::to_string(config_.quality) + ".jpg");

    std::ofstream reportFile(result.reportPath);
    if (!reportFile) {
        throw std::runtime_error("Failed to open report file: " + result.reportPath.string());
    }

    logger.setReportStream(&reportFile);

    auto log = [&](const std::string& message) {
        logger.info(message);
    };

    auto logValue = [&](const std::string& label,
                        auto value,
                        const std::string& suffix = "") {
        std::ostringstream oss;
        oss << label << value << suffix << '\n';
        logger.info(oss.str());
    };

    auto logSeparator = [&]() {
        logger.separator();
    };

    auto logPhaseDuration = [&](const std::chrono::milliseconds& duration) {
        logValue("Phase duration: ", duration.count(), " ms");
        logSeparator();
    };

    logSeparator();
    log("Processing image: " + inputPath.filename().string() + "\n");
    log("Image number: " + imageNumber + "\n");
    logValue("Quality: ", config_.quality);
    logSeparator();

    logger.debug("Output path: " + result.outputPath.string() + "\n");
    logger.debug("Report path: " + result.reportPath.string() + "\n");
    logger.debug("Comparison path: " + result.comparisonPath.string() + "\n");

    const auto cycleStart = std::chrono::high_resolution_clock::now();

    auto start = std::chrono::high_resolution_clock::now();
    log("Loading BMP...\n");
    BmpImage bmp = BmpReader::load(inputPath.string());
    auto end = std::chrono::high_resolution_clock::now();
    logPhaseDuration(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));

    start = std::chrono::high_resolution_clock::now();
    log("Converting RGB to YCbCr...\n");
    YCbCrImage ycbcr = ColorConverter::rgbToYCbCr(bmp.data, bmp.width, bmp.height);
    end = std::chrono::high_resolution_clock::now();
    logPhaseDuration(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));

    const int originalWidth = bmp.width;
    const int originalHeight = bmp.height;
    releaseMemory(bmp);

    start = std::chrono::high_resolution_clock::now();
    log("Padding image to dimensions multiple of 8...\n");
    YCbCrImage padded = Padding::padToMultipleOf8(ycbcr);
    end = std::chrono::high_resolution_clock::now();
    logPhaseDuration(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));

    releaseMemory(ycbcr);

    start = std::chrono::high_resolution_clock::now();
    log("Splitting into 8x8 blocks...\n");
    ImageBlocks blocks = BlockSplitter::splitImageIntoBlocks(padded);
    end = std::chrono::high_resolution_clock::now();
    logPhaseDuration(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));

    start = std::chrono::high_resolution_clock::now();
    log("Applying DCT...\n");
    DctImageBlocks dctBlocks = DCT::applyToImage(blocks);
    end = std::chrono::high_resolution_clock::now();
    logPhaseDuration(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));

    releaseMemory(blocks);
    releaseMemory(padded);

    start = std::chrono::high_resolution_clock::now();
    log("Preparing quantization tables...\n");
    const auto lumaTable = Quantizer::scaledLuminanceTable(config_.quality);
    const auto chromaTable = Quantizer::scaledChrominanceTable(config_.quality);
    end = std::chrono::high_resolution_clock::now();
    logPhaseDuration(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));

    start = std::chrono::high_resolution_clock::now();
    log("Quantizing...\n");
    QuantizedImageBlocks quantized =
        Quantizer::quantizeImage(dctBlocks, lumaTable, chromaTable);
    end = std::chrono::high_resolution_clock::now();
    logPhaseDuration(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));

    releaseMemory(dctBlocks);

    start = std::chrono::high_resolution_clock::now();
    log("Zigzag reorder...\n");
    ZigZagImageBlocks zigzag = ZigZag::reorderImage(quantized);
    end = std::chrono::high_resolution_clock::now();
    logPhaseDuration(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));

    releaseMemory(quantized);

    start = std::chrono::high_resolution_clock::now();
    log("Entropy encoding...\n");
    EntropyImageData entropy = EntropyEncoder::encodeImage(zigzag);
    end = std::chrono::high_resolution_clock::now();
    logPhaseDuration(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));

    releaseMemory(zigzag);

    start = std::chrono::high_resolution_clock::now();
    log("Writing JPEG...\n");
    result.bytesWritten = JpegWriter::writeJpegFile(result.outputPath.string(),
                                                    originalWidth,
                                                    originalHeight,
                                                    lumaTable,
                                                    chromaTable,
                                                    entropy);
    end = std::chrono::high_resolution_clock::now();
    logPhaseDuration(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));

    releaseMemory(entropy);

    const auto cycleEnd = std::chrono::high_resolution_clock::now();
    const auto totalRuntimeMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(cycleEnd - cycleStart);

    log("JPEG written successfully\n");
    log("Input:   " + inputPath.string() + "\n");
    log("Output:  " + result.outputPath.string() + "\n");
    log("Compare: " + result.comparisonPath.string() + "\n");
    log("Report:  " + result.reportPath.string() + "\n");
    logValue("Size:    ", result.bytesWritten, " bytes");
    logSeparator();

    cv::Mat original = cv::imread(inputPath.string(), cv::IMREAD_COLOR);
    cv::Mat compressed = cv::imread(result.outputPath.string(), cv::IMREAD_COLOR);

    if (original.empty()) {
        throw std::runtime_error("Failed to load original image with OpenCV: " +
                                 inputPath.string());
    }

    if (compressed.empty()) {
        throw std::runtime_error("Failed to load compressed JPEG with OpenCV: " +
                                 result.outputPath.string());
    }

    start = std::chrono::high_resolution_clock::now();
    log("Generating visual comparison...\n");

    VisualComparison::createSideBySideComparison(inputPath.string(),
                                                  result.outputPath.string(),
                                                  result.comparisonPath.string(),
                                                  config_.quality);

    end = std::chrono::high_resolution_clock::now();
    logPhaseDuration(std::chrono::duration_cast<std::chrono::milliseconds>(end - start));

    MSE mse = Metrics::computeMSE(original, compressed);

    log("\nMSE:\n");
    logValue("  B: ", mse.mseB);
    logValue("  G: ", mse.mseG);
    logValue("  R: ", mse.mseR);

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
    logger.setReportStream(nullptr);

    result.success = true;
    return result;
}