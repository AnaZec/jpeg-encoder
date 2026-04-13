#include "jpeg_writer.hpp"

#include "bitstream_writer.hpp"
#include "huffman_tables.hpp"

#include <filesystem>
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

uint8_t makeAcSymbol(const EncodedACValue& value) {
    if (value.isEob) {
        return 0x00;
    }

    if (value.isZrl) {
        return 0xF0;
    }

    return static_cast<uint8_t>(((value.runLength & 0x0F) << 4) |
                                (value.size & 0x0F));
}

} // namespace

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
    writeUint16(out, 16);

    out.push_back('J');
    out.push_back('F');
    out.push_back('I');
    out.push_back('F');
    out.push_back(0x00);

    out.push_back(0x01);
    out.push_back(0x01);
    out.push_back(0x00);

    writeUint16(out, 1);
    writeUint16(out, 1);

    out.push_back(0x00);
    out.push_back(0x00);
}

void JpegWriter::writeDQT(std::vector<uint8_t>& out,
                          const std::array<int, 64>& luminanceTable,
                          const std::array<int, 64>& chrominanceTable) {
    writeMarker(out, 0xFFDB);
    writeUint16(out, 132);

    out.push_back(0x00);
    for (int i = 0; i < 64; ++i) {
        out.push_back(static_cast<uint8_t>(luminanceTable[kZigZagIndices[i]]));
    }

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
    writeUint16(out, 17);
    out.push_back(8);
    writeUint16(out, static_cast<uint16_t>(height));
    writeUint16(out, static_cast<uint16_t>(width));
    out.push_back(3);

    // Y component
    out.push_back(1);
    out.push_back(0x11);
    out.push_back(0);

    // Cb component
    out.push_back(2);
    out.push_back(0x11);
    out.push_back(1);

    // Cr component
    out.push_back(3);
    out.push_back(0x11);
    out.push_back(1);
}

void JpegWriter::writeSingleHuffmanTable(std::vector<uint8_t>& out,
                                         uint8_t tableClass,
                                         uint8_t tableId,
                                         const JpegHuffmanTable& table) {
    out.push_back(static_cast<uint8_t>(((tableClass & 0x0F) << 4) |
                                       (tableId & 0x0F)));

    for (uint8_t count : table.codeLengthCounts) {
        out.push_back(count);
    }

    out.insert(out.end(), table.symbols.begin(), table.symbols.end());
}

void JpegWriter::writeDHT(std::vector<uint8_t>& out) {
    std::vector<uint8_t> payload;
    payload.reserve(512);

    // DC table 0: luminance
    writeSingleHuffmanTable(payload, 0, 0, HuffmanTables::luminanceDCTable());

    // AC table 0: luminance
    writeSingleHuffmanTable(payload, 1, 0, HuffmanTables::luminanceACTable());

    // DC table 1: chrominance
    writeSingleHuffmanTable(payload, 0, 1, HuffmanTables::chrominanceDCTable());

    // AC table 1: chrominance
    writeSingleHuffmanTable(payload, 1, 1, HuffmanTables::chrominanceACTable());

    writeMarker(out, 0xFFC4);
    writeUint16(out, static_cast<uint16_t>(payload.size() + 2));
    out.insert(out.end(), payload.begin(), payload.end());
}

void JpegWriter::writeSOS(std::vector<uint8_t>& out) {
    writeMarker(out, 0xFFDA);
    writeUint16(out, 12);

    out.push_back(3);

    // Y uses DC table 0, AC table 0
    out.push_back(1);
    out.push_back(0x00);

    // Cb uses DC table 1, AC table 1
    out.push_back(2);
    out.push_back(0x11);

    // Cr uses DC table 1, AC table 1
    out.push_back(3);
    out.push_back(0x11);

    out.push_back(0);
    out.push_back(63);
    out.push_back(0);
}

void JpegWriter::writeEOI(std::vector<uint8_t>& out) {
    writeMarker(out, 0xFFD9);
}

void JpegWriter::encodeBlockToBitstream(BitstreamWriter& writer,
                                        const EntropyEncodedBlock& block,
                                        const HuffmanCodeMap& dcCodes,
                                        const HuffmanCodeMap& acCodes) {
    const auto dcIt = dcCodes.find(static_cast<uint8_t>(block.dc.category));
    if (dcIt == dcCodes.end()) {
        throw std::runtime_error("Missing Huffman code for DC category");
    }

    writer.writeBits(dcIt->second.code, dcIt->second.length);
    writer.writeBits(block.dc.amplitudeBits);

    for (const auto& acValue : block.acValues) {
        const uint8_t symbol = makeAcSymbol(acValue);

        const auto acIt = acCodes.find(symbol);
        if (acIt == acCodes.end()) {
            throw std::runtime_error("Missing Huffman code for AC symbol");
        }

        writer.writeBits(acIt->second.code, acIt->second.length);

        if (!acValue.isEob && !acValue.isZrl) {
            writer.writeBits(acValue.amplitudeBits);
        }
    }
}

std::vector<uint8_t> JpegWriter::encodeScanData(const EntropyImageData& entropyData) {
    const std::size_t yCount = entropyData.y.blocks.size();
    const std::size_t cbCount = entropyData.cb.blocks.size();
    const std::size_t crCount = entropyData.cr.blocks.size();

    if (yCount != cbCount || yCount != crCount) {
        throw std::runtime_error(
            "Expected equal Y/Cb/Cr block counts for 4:4:4 interleaved baseline JPEG");
    }

    const HuffmanCodeMap lumaDCCodes = HuffmanTables::luminanceDCCodes();
    const HuffmanCodeMap lumaACCodes = HuffmanTables::luminanceACCodes();
    const HuffmanCodeMap chromaDCCodes = HuffmanTables::chrominanceDCCodes();
    const HuffmanCodeMap chromaACCodes = HuffmanTables::chrominanceACCodes();

    BitstreamWriter writer;

    for (std::size_t i = 0; i < yCount; ++i) {
        encodeBlockToBitstream(writer, entropyData.y.blocks[i],  lumaDCCodes,   lumaACCodes);
        encodeBlockToBitstream(writer, entropyData.cb.blocks[i], chromaDCCodes, chromaACCodes);
        encodeBlockToBitstream(writer, entropyData.cr.blocks[i], chromaDCCodes, chromaACCodes);
    }

    writer.flushWithOnes();
    return writer.buffer();
}

std::size_t JpegWriter::writeJpegFile(const std::string& outputPath,
                                      int width,
                                      int height,
                                      const std::array<int, 64>& luminanceTable,
                                      const std::array<int, 64>& chrominanceTable,
                                      const EntropyImageData& entropyData) {
    std::vector<uint8_t> bytes;
    bytes.reserve(4096);

    writeSOI(bytes);
    writeAPP0(bytes);
    writeDQT(bytes, luminanceTable, chrominanceTable);
    writeSOF0(bytes, width, height);
    writeDHT(bytes);
    writeSOS(bytes);

    const std::vector<uint8_t> scanData = encodeScanData(entropyData);
    bytes.insert(bytes.end(), scanData.begin(), scanData.end());

    writeEOI(bytes);

    std::filesystem::path path(outputPath);
    if (!path.parent_path().empty()) {
        std::filesystem::create_directories(path.parent_path());
    }

    std::ofstream file(outputPath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open output JPEG file: " + outputPath);
    }

    file.write(reinterpret_cast<const char*>(bytes.data()),
               static_cast<std::streamsize>(bytes.size()));

    if (!file) {
        throw std::runtime_error("Failed to write JPEG file: " + outputPath);
    }

    return bytes.size();
}