#include "bmp_reader.hpp"

#include <cstdlib>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

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

constexpr uint16_t kBmpSignature = 0x4D42; // "BM"
constexpr uint16_t kSupportedBitDepth = 24;
constexpr uint32_t kBiRgbCompression = 0;
constexpr uint32_t kSupportedInfoHeaderSize = 40;
constexpr int kBytesPerPixel = 3;

void readExact(std::ifstream& file,
               char* destination,
               std::streamsize byteCount,
               const std::string& errorMessage) {
    file.read(destination, byteCount);

    if (file.gcount() != byteCount) {
        throw std::runtime_error(errorMessage);
    }
}

std::streamoff getFileSize(std::ifstream& file, const std::string& path) {
    const std::streampos originalPosition = file.tellg();

    file.seekg(0, std::ios::end);
    if (!file) {
        throw std::runtime_error("Failed to determine file size: " + path);
    }

    const std::streamoff fileSize = file.tellg();

    file.seekg(originalPosition, std::ios::beg);
    if (!file) {
        throw std::runtime_error("Failed to restore file position while reading: " + path);
    }

    return fileSize;
}

void validateImageBufferSize(int width, int height, const std::string& path) {
    const std::size_t w = static_cast<std::size_t>(width);
    const std::size_t h = static_cast<std::size_t>(height);

    if (w == 0 || h == 0) {
        throw std::runtime_error("Invalid BMP dimensions in file: " + path);
    }

    const std::size_t maxSize = std::numeric_limits<std::size_t>::max();

    if (w > maxSize / h / kBytesPerPixel) {
        throw std::runtime_error("BMP image is too large to allocate safely: " + path);
    }
}

} // namespace

BmpImage BmpReader::load(const std::string& path) {
    std::ifstream file(path, std::ios::binary);

    if (!file) {
        throw std::runtime_error("Failed to open BMP file: " + path);
    }

    const std::streamoff fileSize = getFileSize(file, path);

    if (fileSize < static_cast<std::streamoff>(sizeof(BmpFileHeader) + sizeof(BmpInfoHeader))) {
        throw std::runtime_error("File is too small to contain a valid BMP header: " + path);
    }

    BmpFileHeader fileHeader{};
    BmpInfoHeader infoHeader{};

    readExact(file,
              reinterpret_cast<char*>(&fileHeader),
              sizeof(fileHeader),
              "Failed to read BMP file header: " + path);

    readExact(file,
              reinterpret_cast<char*>(&infoHeader),
              sizeof(infoHeader),
              "Failed to read BMP info header: " + path);

    if (fileHeader.bfType != kBmpSignature) {
        throw std::runtime_error("Invalid BMP signature in file: " + path);
    }

    if (infoHeader.biSize != kSupportedInfoHeaderSize) {
        throw std::runtime_error(
            "Unsupported BMP DIB header size in file: " + path +
            ". Only BITMAPINFOHEADER 40-byte headers are supported."
        );
    }

    if (infoHeader.biPlanes != 1) {
        throw std::runtime_error("Invalid BMP planes count in file: " + path);
    }

    if (infoHeader.biBitCount != kSupportedBitDepth) {
        throw std::runtime_error(
            "Unsupported BMP bit depth in file: " + path +
            ". Only 24-bit BMP files are supported."
        );
    }

    if (infoHeader.biCompression != kBiRgbCompression) {
        throw std::runtime_error(
            "Unsupported compressed BMP file: " + path +
            ". Only uncompressed BI_RGB BMP files are supported."
        );
    }

    if (infoHeader.biWidth <= 0 || infoHeader.biHeight == 0) {
        throw std::runtime_error("Invalid BMP dimensions in file: " + path);
    }

    if (fileHeader.bfOffBits < sizeof(BmpFileHeader) + sizeof(BmpInfoHeader)) {
        throw std::runtime_error("Invalid BMP pixel data offset in file: " + path);
    }

    if (fileHeader.bfOffBits >= static_cast<uint32_t>(fileSize)) {
        throw std::runtime_error("BMP pixel data offset points beyond end of file: " + path);
    }

    const int width = infoHeader.biWidth;
    const int height = std::abs(infoHeader.biHeight);
    const bool bottomUp = (infoHeader.biHeight > 0);

    validateImageBufferSize(width, height, path);

    const int rowSizeOnDisk = ((width * kBytesPerPixel + 3) / 4) * 4;

    const std::streamoff expectedPixelBytes =
        static_cast<std::streamoff>(rowSizeOnDisk) *
        static_cast<std::streamoff>(height);

    if (static_cast<std::streamoff>(fileHeader.bfOffBits) + expectedPixelBytes > fileSize) {
        throw std::runtime_error("BMP pixel data is truncated or corrupt: " + path);
    }

    std::vector<uint8_t> rowBuffer(static_cast<std::size_t>(rowSizeOnDisk));

    std::vector<uint8_t> rgbData(static_cast<std::size_t>(width) *
                                 static_cast<std::size_t>(height) *
                                 kBytesPerPixel);

    file.seekg(fileHeader.bfOffBits, std::ios::beg);
    if (!file) {
        throw std::runtime_error("Failed to seek to BMP pixel data: " + path);
    }

    for (int row = 0; row < height; ++row) {
        readExact(file,
                  reinterpret_cast<char*>(rowBuffer.data()),
                  rowSizeOnDisk,
                  "Failed to read BMP pixel row. File may be truncated: " + path);

        const int destinationRow = bottomUp ? (height - 1 - row) : row;

        for (int col = 0; col < width; ++col) {
            const int sourceIndex = col * kBytesPerPixel;
            const int destinationIndex =
                (destinationRow * width + col) * kBytesPerPixel;

            // BMP stores 24-bit pixels as BGR. Internally, the encoder uses RGB.
            rgbData[destinationIndex + 0] = rowBuffer[sourceIndex + 2];
            rgbData[destinationIndex + 1] = rowBuffer[sourceIndex + 1];
            rgbData[destinationIndex + 2] = rowBuffer[sourceIndex + 0];
        }
    }

    return BmpImage{width, height, std::move(rgbData)};
}