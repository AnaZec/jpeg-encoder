#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

class JpegWriter {
public:
    static void writeJpegSkeleton(const std::string& outputPath,
                                  int width,
                                  int height,
                                  const std::array<int, 64>& luminanceTable,
                                  const std::array<int, 64>& chrominanceTable);

private:
    static void writeMarker(std::vector<uint8_t>& out, uint16_t marker);
    static void writeUint16(std::vector<uint8_t>& out, uint16_t value);

    static void writeSOI(std::vector<uint8_t>& out);
    static void writeAPP0(std::vector<uint8_t>& out);
    static void writeDQT(std::vector<uint8_t>& out,
                         const std::array<int, 64>& luminanceTable,
                         const std::array<int, 64>& chrominanceTable);
    static void writeSOF0(std::vector<uint8_t>& out, int width, int height);
    static void writeDHTPlaceholder(std::vector<uint8_t>& out);
    static void writeSOS(std::vector<uint8_t>& out);
    static void writeEOI(std::vector<uint8_t>& out);
}; 