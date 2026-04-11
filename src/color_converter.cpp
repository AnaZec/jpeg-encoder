#include "color_converter.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
uint8_t clampToByte(double value) {
    value = std::round(value);
    value = std::clamp(value, 0.0, 255.0);
    return static_cast<uint8_t>(value);
}
}

YCbCrImage ColorConverter::rgbToYCbCr(const std::vector<uint8_t>& rgbData, int width, int height) {
    if (width <= 0 || height <= 0) {
        throw std::runtime_error("Invalid image dimensions for RGB to YCbCr conversion");
    }

    const std::size_t expectedSize = static_cast<std::size_t>(width) * height * 3;
    if (rgbData.size() != expectedSize) {
        throw std::runtime_error("RGB buffer size does not match width * height * 3");
    }

    YCbCrImage result;
    result.width = width;
    result.height = height;
    result.y.resize(static_cast<std::size_t>(width) * height);
    result.cb.resize(static_cast<std::size_t>(width) * height);
    result.cr.resize(static_cast<std::size_t>(width) * height);

    for (std::size_t pixel = 0, j = 0; pixel < expectedSize; pixel += 3, ++j) {
        const double r = static_cast<double>(rgbData[pixel]);
        const double g = static_cast<double>(rgbData[pixel + 1]);
        const double b = static_cast<double>(rgbData[pixel + 2]);

        const double y  =  0.299000 * r + 0.587000 * g + 0.114000 * b;
        const double cb = -0.168736 * r - 0.331264 * g + 0.500000 * b + 128.0;
        const double cr =  0.500000 * r - 0.418688 * g - 0.081312 * b + 128.0;

        result.y[j]  = clampToByte(y);
        result.cb[j] = clampToByte(cb);
        result.cr[j] = clampToByte(cr);
    }

    return result;
}