#include "encoder_pipeline.hpp"
#include "logger.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static std::string toLower(std::string value) {
    std::transform(value.begin(),
                   value.end(),
                   value.begin(),
                   [](unsigned char c) {
                       return static_cast<char>(std::tolower(c));
                   });

    return value;
}

static LogLevel parseLogLevel(const std::string& value) {
    const std::string level = toLower(value);

    if (level == "error") {
        return LogLevel::Error;
    }

    if (level == "info") {
        return LogLevel::Info;
    }

    if (level == "debug") {
        return LogLevel::Debug;
    }

    throw std::runtime_error(
        "Invalid log level: '" + value + "'. Expected: error, info, or debug."
    );
}

struct AppConfig {
    int quality = 75;
    LogLevel logLevel = Logger::defaultLevel();
};

static AppConfig parseArguments(int argc, char* argv[]) {
    AppConfig config;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "--quality") {
            if (i + 1 >= argc) {
                throw std::runtime_error(
                    "Missing value for --quality. Expected integer in range [1, 100]."
                );
            }

            const std::string value = argv[++i];

            std::size_t consumed = 0;
            try {
                config.quality = std::stoi(value, &consumed);
            } catch (const std::exception&) {
                throw std::runtime_error(
                    "Invalid value for --quality: '" + value +
                    "'. Expected integer in range [1, 100]."
                );
            }

            if (consumed != value.size()) {
                throw std::runtime_error(
                    "Invalid value for --quality: '" + value +
                    "'. Expected integer without extra characters."
                );
            }
        } else if (arg == "--log-level") {
            if (i + 1 >= argc) {
                throw std::runtime_error(
                    "Missing value for --log-level. Expected: error, info, or debug."
                );
            }

            config.logLevel = parseLogLevel(argv[++i]);
        } else if (arg == "--help" || arg == "-h") {
            std::cout
                << "Usage: ./jpeg_encoder [--quality <1-100>] "
                << "[--log-level <error|info|debug>]\n\n"
                << "Options:\n"
                << "  --quality <1-100>              JPEG quality factor. Default: 75\n"
                << "  --log-level <error|info|debug> Logging verbosity\n"
                << "  --help, -h                     Show this help message\n";
            std::exit(0);
        } else {
            throw std::runtime_error(
                "Unknown argument: " + arg +
                "\nUsage: ./jpeg_encoder [--quality <1-100>] "
                "[--log-level <error|info|debug>]"
            );
        }
    }

    if (config.quality < 1 || config.quality > 100) {
        throw std::runtime_error(
            "Quality must be in range [1, 100]. Received: " +
            std::to_string(config.quality)
        );
    }

    return config;
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
        const AppConfig config = parseArguments(argc, argv);
        const int quality = config.quality;

        Logger logger(config.logLevel, std::cout, std::cerr);

        const fs::path inputDir = "../images/input";
        const fs::path outputDir = "../images/output";
        const fs::path reportDir = "../reports";
        const fs::path comparisonDir = "../images/comparisons";

        fs::create_directories(outputDir);
        fs::create_directories(reportDir);
        fs::create_directories(comparisonDir);

        if (!fs::exists(inputDir) || !fs::is_directory(inputDir)) {
            throw std::runtime_error("Input directory does not exist: " + inputDir.string());
        }

        std::vector<fs::path> bmpFiles;
        for (const auto& entry : fs::directory_iterator(inputDir)) {
            if (!entry.is_regular_file()) {
                continue;
            }

            const std::string ext = toLower(entry.path().extension().string());
            if (ext == ".bmp") {
                bmpFiles.push_back(entry.path());
            }
        }

        if (bmpFiles.empty()) {
            throw std::runtime_error("No BMP files found in: " + inputDir.string());
        }

        std::sort(bmpFiles.begin(), bmpFiles.end());

        {
            std::ostringstream oss;
            oss << "Found " << bmpFiles.size() << " BMP image(s) in "
                << inputDir.string() << '\n';
            logger.info(oss.str());
        }

        logger.info("Using quality = " + std::to_string(quality) + "\n\n");

        logger.debug("Log level: debug diagnostics enabled\n");
        logger.debug("Sorted BMP input list:\n");
        for (const auto& bmpFile : bmpFiles) {
            logger.debug("  " + bmpFile.string() + "\n");
        }

        EncoderPipelineConfig pipelineConfig{
            quality,
            outputDir,
            reportDir,
            comparisonDir
        };

        EncoderPipeline pipeline(pipelineConfig);

        int processedCount = 0;
        int failedCount = 0;

        for (std::size_t i = 0; i < bmpFiles.size(); ++i) {
            const fs::path& inputPath = bmpFiles[i];
            const std::string imageNumber =
                extractImageNumber(inputPath, static_cast<int>(i + 1));

            try {
                const EncoderPipelineResult result =
                    pipeline.processImage(inputPath, imageNumber, logger);

                if (result.success) {
                    ++processedCount;
                }
            } catch (const std::exception& e) {
                logger.error("Error while processing " + inputPath.filename().string() +
                             ": " + e.what() + "\n");
                logger.setReportStream(nullptr);
                ++failedCount;
            }
        }

        logger.info("========================================\n");
        logger.info("Batch processing finished.\n");
        logger.info("Processed successfully: " + std::to_string(processedCount) + "\n");
        logger.info("Failed: " + std::to_string(failedCount) + "\n");
        logger.info("========================================\n");

        if (processedCount == 0) {
            return 1;
        }

        if (failedCount > 0) {
            return 2;
        }

    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << '\n';
        return 1;
    }

    return 0;
}