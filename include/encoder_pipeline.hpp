#pragma once

#include "logger.hpp"

#include <filesystem>
#include <string>

struct EncoderPipelineConfig {
    int quality = 75;
    std::filesystem::path outputDir;
    std::filesystem::path reportDir;
    std::filesystem::path comparisonDir;
};

struct EncoderPipelineResult {
    bool success = false;
    std::filesystem::path inputPath;
    std::filesystem::path outputPath;
    std::filesystem::path reportPath;
    std::filesystem::path comparisonPath;
    std::size_t bytesWritten = 0;
};

class EncoderPipeline {
public:
    explicit EncoderPipeline(EncoderPipelineConfig config);

    EncoderPipelineResult processImage(const std::filesystem::path& inputPath,
                                       const std::string& imageNumber,
                                       Logger& logger) const;

private:
    EncoderPipelineConfig config_;
};