#ifndef ENABLE_HUFFMAN_DEBUG
#define ENABLE_HUFFMAN_DEBUG 0
#endif

#include "jpeg_writer.hpp"

#include "bitstream_writer.hpp"
#include "huffman_tables.hpp"
#include "huffman_generator.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <iomanip>
#include <iostream>

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

void debugPrintHuffmanTable(const std::string& name, const JpegHuffmanTable& table) {
    std::cout << "\n[DEBUG] Huffman table: " << name << "\n";

    std::size_t totalSymbolsFromCounts = 0;

    std::cout << "[DEBUG] Code length counts:";
    for (std::size_t i = 0; i < table.codeLengthCounts.size(); ++i) {
        const auto count = table.codeLengthCounts[i];
        totalSymbolsFromCounts += count;

        if (count > 0) {
            std::cout << " L" << (i + 1) << "=" << static_cast<int>(count);
        }
    }
    std::cout << "\n";

    std::cout << "[DEBUG] Symbols vector size: " << table.symbols.size() << "\n";
    std::cout << "[DEBUG] Total symbols from counts: " << totalSymbolsFromCounts << "\n";

    if (totalSymbolsFromCounts != table.symbols.size()) {
        std::cout << "[DEBUG][ERROR] DHT count/symbol mismatch in " << name << "\n";
    }

    std::cout << "[DEBUG] First symbols:";
    const std::size_t previewCount = std::min<std::size_t>(table.symbols.size(), 20);
    for (std::size_t i = 0; i < previewCount; ++i) {
        std::cout << " 0x"
                  << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(table.symbols[i])
                  << std::dec << std::setfill(' ');
    }
    std::cout << "\n";
}

void debugPrintCodeMap(const std::string& name, const HuffmanCodeMap& codes) {
    std::cout << "\n[DEBUG] Huffman code map: " << name << "\n";
    std::cout << "[DEBUG] Code count: " << codes.size() << "\n";

    std::size_t previewed = 0;
    for (const auto& [symbol, code] : codes) {
        if (previewed >= 20) {
            break;
        }

        std::cout << "[DEBUG] symbol=0x"
                  << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(symbol)
                  << std::dec << std::setfill(' ')
                  << " length=" << static_cast<int>(code.length)
                  << " code=0b";

        for (int bit = code.length - 1; bit >= 0; --bit) {
            std::cout << ((code.code >> bit) & 1);
        }

        std::cout << "\n";
        ++previewed;
    }
}

void debugValidateTable(const std::string& name, const JpegHuffmanTable& table) {
    std::size_t totalSymbolsFromCounts = 0;

    for (uint8_t count : table.codeLengthCounts) {
        totalSymbolsFromCounts += count;
    }

    if (totalSymbolsFromCounts != table.symbols.size()) {
        std::cout << "[DEBUG][ERROR] Invalid table " << name
                  << ": counts say " << totalSymbolsFromCounts
                  << " symbols, but vector has " << table.symbols.size()
                  << "\n";
    }

    if (table.symbols.empty()) {
        std::cout << "[DEBUG][ERROR] Invalid table " << name
                  << ": no symbols\n";
    }

    if (table.symbols.size() > 255) {
        std::cout << "[DEBUG][ERROR] Invalid table " << name
                  << ": too many symbols\n";
    }
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

void JpegWriter::writeDHT(std::vector<uint8_t>& out,
                          const JpegHuffmanTableSet& huffmanTables) {
    std::vector<uint8_t> payload;
    payload.reserve(512);

    // DC table 0: luminance
    writeSingleHuffmanTable(payload, 0, 0, huffmanTables.luminanceDC);

    // AC table 0: luminance
    writeSingleHuffmanTable(payload, 1, 0, huffmanTables.luminanceAC);

    // DC table 1: chrominance
    writeSingleHuffmanTable(payload, 0, 1, huffmanTables.chrominanceDC);

    // AC table 1: chrominance
    writeSingleHuffmanTable(payload, 1, 1, huffmanTables.chrominanceAC);

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
    const auto dcSymbol = static_cast<uint8_t>(block.dc.category);
    const auto dcCodeIt = dcCodes.find(dcSymbol);
    if (dcCodeIt == dcCodes.end()) {
        std::cout << "[DEBUG][ERROR] Missing DC Huffman code for symbol 0x"
                << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(dcSymbol)
                << std::dec << std::setfill(' ')
                << " category=" << block.dc.category << "\n";

        throw std::runtime_error("Missing DC Huffman code");
    }

    writer.writeBits(dcCodeIt->second.code, dcCodeIt->second.length);
    writer.writeBits(block.dc.amplitudeBits);

    for (const auto& acValue : block.acValues) {
        const uint8_t symbol = makeAcSymbol(acValue);

        const auto acIt = acCodes.find(symbol);
        if (acIt == acCodes.end()) {
            std::cout << "[DEBUG][ERROR] Missing Huffman code for AC symbol 0x"
                    << std::hex << std::setw(2) << std::setfill('0')
                    << static_cast<int>(symbol)
                    << std::dec << std::setfill(' ')
                    << "\n";

            throw std::runtime_error("Missing Huffman code for AC symbol");
        }

        writer.writeBits(acIt->second.code, acIt->second.length);

        if (!acValue.isEob && !acValue.isZrl) {
            writer.writeBits(acValue.amplitudeBits);
        }
    }
}

std::vector<uint8_t> JpegWriter::encodeScanData(const EntropyImageData& entropyData,
                                                const JpegHuffmanCodeSet& huffmanCodes) {
    const std::size_t yCount = entropyData.y.blocks.size();
    const std::size_t cbCount = entropyData.cb.blocks.size();
    const std::size_t crCount = entropyData.cr.blocks.size();

    if (yCount != cbCount || yCount != crCount) {
        throw std::runtime_error(
            "Expected equal Y/Cb/Cr block counts for 4:4:4 interleaved baseline JPEG");
    }

    BitstreamWriter writer;

    for (std::size_t i = 0; i < yCount; ++i) {
        encodeBlockToBitstream(writer,
                               entropyData.y.blocks[i],
                               huffmanCodes.luminanceDC,
                               huffmanCodes.luminanceAC);

        encodeBlockToBitstream(writer,
                               entropyData.cb.blocks[i],
                               huffmanCodes.chrominanceDC,
                               huffmanCodes.chrominanceAC);

        encodeBlockToBitstream(writer,
                               entropyData.cr.blocks[i],
                               huffmanCodes.chrominanceDC,
                               huffmanCodes.chrominanceAC);
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

    const JpegHuffmanTableSet huffmanTables =
    HuffmanGenerator::generateFromEntropyData(entropyData);

    const JpegHuffmanCodeSet huffmanCodes =
        HuffmanTables::buildCodeSet(huffmanTables);

    #if ENABLE_HUFFMAN_DEBUG
    std::cout << "\n========== HUFFMAN DEBUG ==========\n";

    debugValidateTable("Luminance DC", huffmanTables.luminanceDC);
    debugValidateTable("Luminance AC", huffmanTables.luminanceAC);
    debugValidateTable("Chrominance DC", huffmanTables.chrominanceDC);
    debugValidateTable("Chrominance AC", huffmanTables.chrominanceAC);

    debugPrintHuffmanTable("Luminance DC", huffmanTables.luminanceDC);
    debugPrintHuffmanTable("Luminance AC", huffmanTables.luminanceAC);
    debugPrintHuffmanTable("Chrominance DC", huffmanTables.chrominanceDC);
    debugPrintHuffmanTable("Chrominance AC", huffmanTables.chrominanceAC);

    debugPrintCodeMap("Luminance DC", huffmanCodes.luminanceDC);
    debugPrintCodeMap("Luminance AC", huffmanCodes.luminanceAC);
    debugPrintCodeMap("Chrominance DC", huffmanCodes.chrominanceDC);
    debugPrintCodeMap("Chrominance AC", huffmanCodes.chrominanceAC);

    std::cout << "======== END HUFFMAN DEBUG ========\n\n";
    #endif

    writeSOI(bytes);
    writeAPP0(bytes);
    writeDQT(bytes, luminanceTable, chrominanceTable);
    writeSOF0(bytes, width, height);
    writeDHT(bytes, huffmanTables);
    writeSOS(bytes);

    const std::vector<uint8_t> scanData = encodeScanData(entropyData, huffmanCodes);
    
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