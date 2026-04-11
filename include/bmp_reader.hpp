#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct BmpImage {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> data; // Stored as RGB, 3 bytes per pixel
};

class BmpReader {
public:
    static BmpImage load(const std::string& path);
};