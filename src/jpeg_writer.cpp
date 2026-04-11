#include "jpeg_writer.hpp"

#include <fstream>
#include <stdexcept>

namespace {

constexpr std::array<int, 64> kZigZagIndices = {
     0,  1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

}

void JpegWriter::writeUint16(std::vector<uint8_t>& out, uint16_t value) {
    out.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(value & 0xFF));
}

void JpegWriter::writeMarker(std::vector<uint8_t>& out, uint16_t marker) {
    writeUint16(out, marker);
}

void JpegWriter::writeSOI(std::vector<uint8_t>& out) {
    writeMarker(out, 0xFFD8);
}

void JpegWriter::writeAPP0(std::vector<uint8_t>& out) {
    writeMarker(out, 0xFFE0);
    writeUint16(out, 16); // segment length

    // "JFIF\0"
    out.push_back('J');
    out.push_back('F');
    out.push_back('I');
    out.push_back('F');
    out.push_back(0x00);

    out.push_back(0x01); // version major
    out.push_back(0x01); // version minor
    out.push_back(0x00); // density units: none

    writeUint16(out, 1); // X density
    writeUint16(out, 1); // Y density

    out.push_back(0x00); // thumbnail width
    out.push_back(0x00); // thumbnail height
}

void JpegWriter::writeDQT(std::vector<uint8_t>& out,
                          const std::array<int, 64>& luminanceTable,
                          const std::array<int, 64>& chrominanceTable) {
    writeMarker(out, 0xFFDB);

    // 2 bytes length + (1 info + 64 values) * 2 tables = 132
    writeUint16(out, 132);

    // Table 0: luminance, 8-bit precision
    out.push_back(0x00);
    for (int i = 0; i < 64; ++i) {
        out.push_back(static_cast<uint8_t>(luminanceTable[kZigZagIndices[i]]));
    }

    // Table 1: chrominance, 8-bit precision
    out.push_back(0x01);
    for (int i = 0; i < 64; ++i) {
        out.push_back(static_cast<uint8_t>(chrominanceTable[kZigZagIndices[i]]));
    }
}

void JpegWriter::writeSOF0(std::vector<uint8_t>& out, int width, int height) {
    if (width <= 0 || height <= 0 || width > 65535 || height > 65535) {
        throw std::runtime_error("Invalid JPEG dimensions");
    }

    writeMarker(out, 0xFFC0);
    writeUint16(out, 17);   // baseline SOF0 length
    out.push_back(8);       // sample precision
    writeUint16(out, static_cast<uint16_t>(height));
    writeUint16(out, static_cast<uint16_t>(width));
    out.push_back(3);       // number of components

    // Component 1: Y
    out.push_back(1);       // component ID
    out.push_back(0x11);    // sampling factors H=1, V=1
    out.push_back(0);       // quant table 0

    // Component 2: Cb
    out.push_back(2);
    out.push_back(0x11);
    out.push_back(1);       // quant table 1

    // Component 3: Cr
    out.push_back(3);
    out.push_back(0x11);
    out.push_back(1);       // quant table 1
}

void JpegWriter::writeDHTPlaceholder(std::vector<uint8_t>& out) {
    // Placeholder minimal DHT segment.
    writeMarker(out, 0xFFC4);

    // Minimal placeholder payload:
    // length = 2 + 1 + 16 + 1 = 20
    writeUint16(out, 20);

    out.push_back(0x00); // class/id: DC table 0

    // 16 code length counts
    out.push_back(1);
    for (int i = 1; i < 16; ++i) {
        out.push_back(0);
    }

    // one symbol
    out.push_back(0);
}

void JpegWriter::writeSOS(std::vector<uint8_t>& out) {
    writeMarker(out, 0xFFDA);
    writeUint16(out, 12); // length

    out.push_back(3); // number of components in scan

    // Y uses DC table 0, AC table 0
    out.push_back(1);
    out.push_back(0x00);

    // Cb uses DC table 0, AC table 0 for placeholder phase
    out.push_back(2);
    out.push_back(0x00);

    // Cr uses DC table 0, AC table 0 for placeholder phase
    out.push_back(3);
    out.push_back(0x00);

    out.push_back(0);   // Ss
    out.push_back(63);  // Se
    out.push_back(0);   // Ah/Al
}

void JpegWriter::writeEOI(std::vector<uint8_t>& out) {
    writeMarker(out, 0xFFD9);
}

void JpegWriter::writeJpegSkeleton(const std::string& outputPath,
                                   int width,
                                   int height,
                                   const std::array<int, 64>& luminanceTable,
                                   const std::array<int, 64>& chrominanceTable) {
    std::vector<uint8_t> bytes;
    bytes.reserve(512);

    writeSOI(bytes);
    writeAPP0(bytes);
    writeDQT(bytes, luminanceTable, chrominanceTable);
    writeSOF0(bytes, width, height);
    writeDHTPlaceholder(bytes);
    writeSOS(bytes);

    // Temporary placeholder scan data
    // Later this must be replaced with the real Huffman-coded entropy stream.
    bytes.push_back(0x00);

    writeEOI(bytes);

    std::ofstream file(outputPath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open output JPEG file: " + outputPath);
    }

    file.write(reinterpret_cast<const char*>(bytes.data()),
               static_cast<std::streamsize>(bytes.size()));

    if (!file) {
        throw std::runtime_error("Failed to write JPEG file: " + outputPath);
    }
}