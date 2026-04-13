#pragma once

#include "bitstream_writer.hpp"
#include "entropy_encoder.hpp"
#include "huffman_tables.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class JpegWriter {
public:
    static std::size_t writeJpegFile(const std::string& outputPath,
                                     int width,
                                     int height,
                                     const std::array<int, 64>& luminanceTable,
                                     const std::array<int, 64>& chrominanceTable,
                                     const EntropyImageData& entropyData);

private:
    static void writeMarker(std::vector<uint8_t>& out, uint16_t marker);
    static void writeUint16(std::vector<uint8_t>& out, uint16_t value);

    static void writeSOI(std::vector<uint8_t>& out);
    static void writeAPP0(std::vector<uint8_t>& out);
    static void writeDQT(std::vector<uint8_t>& out,
                         const std::array<int, 64>& luminanceTable,
                         const std::array<int, 64>& chrominanceTable);
    static void writeSOF0(std::vector<uint8_t>& out, int width, int height);
    static void writeDHT(std::vector<uint8_t>& out);
    static void writeSOS(std::vector<uint8_t>& out);
    static void writeEOI(std::vector<uint8_t>& out);

    static void writeSingleHuffmanTable(std::vector<uint8_t>& out,
                                        uint8_t tableClass,
                                        uint8_t tableId,
                                        const JpegHuffmanTable& table);

    static std::vector<uint8_t> encodeScanData(const EntropyImageData& entropyData);

    static void encodeBlockToBitstream(BitstreamWriter& writer,
                                       const EntropyEncodedBlock& block,
                                       const HuffmanCodeMap& dcCodes,
                                       const HuffmanCodeMap& acCodes);
};