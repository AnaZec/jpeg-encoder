#include "metrics.hpp"

#include <cmath>
#include <stdexcept>

MSE Metrics::computeMSE(const cv::Mat& original, const cv::Mat& compressed) {
    if (original.empty() || compressed.empty()) {
        throw std::runtime_error("Images must not be empty");
    }

    if (original.size() != compressed.size()) {
        throw std::runtime_error("Images must have the same dimensions");
    }

    if (original.type() != CV_8UC3 || compressed.type() != CV_8UC3) {
        throw std::runtime_error("Only 3-channel 8-bit images are supported");
    }

    double mseB = 0.0;
    double mseG = 0.0;
    double mseR = 0.0;

    const int rows = original.rows;
    const int cols = original.cols;

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            const cv::Vec3b orig = original.at<cv::Vec3b>(y, x);
            const cv::Vec3b comp = compressed.at<cv::Vec3b>(y, x);

            const double db = static_cast<double>(orig[0]) - static_cast<double>(comp[0]);
            const double dg = static_cast<double>(orig[1]) - static_cast<double>(comp[1]);
            const double dr = static_cast<double>(orig[2]) - static_cast<double>(comp[2]);

            mseB += db * db;
            mseG += dg * dg;
            mseR += dr * dr;
        }
    }

    const double totalPixels = static_cast<double>(rows * cols);

    MSE result;
    result.mseB = mseB / totalPixels;
    result.mseG = mseG / totalPixels;
    result.mseR = mseR / totalPixels;
    return result;
}

PSNR Metrics::computePSNR(const MSE& mse) {
    const double maxValue = 255.0;

    auto calc = [maxValue](double mseValue) -> double {
        if (mseValue == 0.0) {
            return 100.0;
        }
        return 10.0 * std::log10((maxValue * maxValue) / mseValue);
    };

    PSNR result;
    result.psnrB = calc(mse.mseB);
    result.psnrG = calc(mse.mseG);
    result.psnrR = calc(mse.mseR);
    return result;
}