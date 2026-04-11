#include "bmp_reader.hpp"

#include <fstream>
#include <stdexcept>
#include <vector>
#include <cstdlib>

namespace {
#pragma pack(push, 1)
struct BmpFileHeader {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
};

struct BmpInfoHeader {
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};
#pragma pack(pop)
}

BmpImage BmpReader::load(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open BMP file: " + path);
    }

    BmpFileHeader fileHeader{};
    BmpInfoHeader infoHeader{};

    file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    file.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));

    if (!file) {
        throw std::runtime_error("Failed to read BMP headers: " + path);
    }

    if (fileHeader.bfType != 0x4D42) {
        throw std::runtime_error("Invalid BMP signature: " + path);
    }

    if (infoHeader.biSize != 40) {
        throw std::runtime_error("Unsupported BMP DIB header size");
    }

    if (infoHeader.biPlanes != 1) {
        throw std::runtime_error("Invalid BMP planes count");
    }

    if (infoHeader.biBitCount != 24) {
        throw std::runtime_error("Only 24-bit BMP files are supported");
    }

    if (infoHeader.biCompression != 0) {
        throw std::runtime_error("Compressed BMP files are not supported");
    }

    if (infoHeader.biWidth <= 0 || infoHeader.biHeight == 0) {
        throw std::runtime_error("Invalid BMP dimensions");
    }

    const int width = infoHeader.biWidth;
    const int height = std::abs(infoHeader.biHeight);
    const bool bottomUp = (infoHeader.biHeight > 0);

    const int bytesPerPixel = 3;
    const int rowSizeOnDisk = ((width * bytesPerPixel + 3) / 4) * 4;

    std::vector<uint8_t> rowBuffer(rowSizeOnDisk);
    std::vector<uint8_t> rgbData(static_cast<size_t>(width) * height * bytesPerPixel);

    file.seekg(fileHeader.bfOffBits, std::ios::beg);
    if (!file) {
        throw std::runtime_error("Failed to seek to BMP pixel data");
    }

    for (int row = 0; row < height; ++row) {
        file.read(reinterpret_cast<char*>(rowBuffer.data()), rowSizeOnDisk);
        if (!file) {
            throw std::runtime_error("Failed to read BMP pixel row");
        }

        const int dstRow = bottomUp ? (height - 1 - row) : row;

        for (int col = 0; col < width; ++col) {
            const int srcIdx = col * 3;
            const int dstIdx = (dstRow * width + col) * 3;

            // BMP stores BGR, convert to RGB
            rgbData[dstIdx + 0] = rowBuffer[srcIdx + 2];
            rgbData[dstIdx + 1] = rowBuffer[srcIdx + 1];
            rgbData[dstIdx + 2] = rowBuffer[srcIdx + 0];
        }
    }

    return BmpImage{width, height, std::move(rgbData)};
}