#include "visual_comparison.hpp"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <stdexcept>
#include <string>

namespace {

constexpr int kHeaderHeight = 48;
constexpr int kSeparatorWidth = 8;

cv::Mat resizeToHeight(const cv::Mat& image, int targetHeight) {
    if (image.empty()) {
        throw std::runtime_error("Cannot resize empty image for visual comparison");
    }

    const double scale = static_cast<double>(targetHeight) / image.rows;
    const int targetWidth = static_cast<int>(image.cols * scale);

    cv::Mat resized;
    cv::resize(image, resized, cv::Size(targetWidth, targetHeight), 0, 0, cv::INTER_AREA);

    return resized;
}

void drawLabel(cv::Mat& canvas,
               const std::string& label,
               int x,
               int y) {
    cv::putText(canvas,
                label,
                cv::Point(x, y),
                cv::FONT_HERSHEY_SIMPLEX,
                0.8,
                cv::Scalar(255, 255, 255),
                2,
                cv::LINE_AA);
}

} // namespace

void VisualComparison::createSideBySideComparison(const std::string& originalPath,
                                                  const std::string& compressedPath,
                                                  const std::string& outputPath,
                                                  int quality) {
    cv::Mat original = cv::imread(originalPath, cv::IMREAD_COLOR);
    cv::Mat compressed = cv::imread(compressedPath, cv::IMREAD_COLOR);

    if (original.empty()) {
        throw std::runtime_error("Failed to load original image for comparison: " + originalPath);
    }

    if (compressed.empty()) {
        throw std::runtime_error("Failed to load compressed image for comparison: " + compressedPath);
    }

    const int comparisonHeight = std::min(original.rows, compressed.rows);

    cv::Mat originalResized = resizeToHeight(original, comparisonHeight);
    cv::Mat compressedResized = resizeToHeight(compressed, comparisonHeight);

    const int canvasWidth =
        originalResized.cols + kSeparatorWidth + compressedResized.cols;

    const int canvasHeight = comparisonHeight + kHeaderHeight;

    cv::Mat canvas(canvasHeight,
                   canvasWidth,
                   CV_8UC3,
                   cv::Scalar(35, 35, 35));

    cv::Rect originalRoi(0,
                         kHeaderHeight,
                         originalResized.cols,
                         originalResized.rows);

    cv::Rect compressedRoi(originalResized.cols + kSeparatorWidth,
                           kHeaderHeight,
                           compressedResized.cols,
                           compressedResized.rows);

    originalResized.copyTo(canvas(originalRoi));
    compressedResized.copyTo(canvas(compressedRoi));

    drawLabel(canvas, "Original BMP", 16, 32);
    drawLabel(canvas,
              "Compressed JPEG - Q=" + std::to_string(quality),
              originalResized.cols + kSeparatorWidth + 16,
              32);

    if (!cv::imwrite(outputPath, canvas)) {
        throw std::runtime_error("Failed to write visual comparison: " + outputPath);
    }
}