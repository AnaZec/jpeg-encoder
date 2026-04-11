#pragma once

#include <cstdint>
#include <vector>

struct YCbCrImage {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> y;
    std::vector<uint8_t> cb;
    std::vector<uint8_t> cr;
};

class ColorConverter {
public:
    static YCbCrImage rgbToYCbCr(const std::vector<uint8_t>& rgbData, int width, int height);
};