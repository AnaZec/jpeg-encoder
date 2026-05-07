#include "huffman_generator.hpp"

#include <algorithm>
#include <memory>
#include <queue>
#include <stdexcept>
#include <vector>

namespace {

struct HuffmanNode {
    uint32_t frequency = 0;
    int symbol = -1;
    std::shared_ptr<HuffmanNode> left = nullptr;
    std::shared_ptr<HuffmanNode> right = nullptr;

    bool isLeaf() const {
        return !left && !right;
    }
};

struct NodeCompare {
    bool operator()(const std::shared_ptr<HuffmanNode>& a,
                    const std::shared_ptr<HuffmanNode>& b) const {
        if (a->frequency != b->frequency) {
            return a->frequency > b->frequency;
        }

        return a->symbol > b->symbol;
    }
};

void collectCodeLengths(const std::shared_ptr<HuffmanNode>& node,
                        uint16_t depth,
                        std::vector<std::pair<uint8_t, uint16_t>>& symbolLengths) {
    if (!node) {
        return;
    }

    if (node->isLeaf()) {
        const uint16_t length = depth == 0 ? 1 : depth;
        symbolLengths.emplace_back(static_cast<uint8_t>(node->symbol), length);
        return;
    }

    collectCodeLengths(node->left, static_cast<uint16_t>(depth + 1), symbolLengths);
    collectCodeLengths(node->right, static_cast<uint16_t>(depth + 1), symbolLengths);
}

std::array<uint16_t, 257> buildCodeLengthCounts(
    const std::vector<std::pair<uint8_t, uint16_t>>& symbolLengths) {
    std::array<uint16_t, 257> lengthCounts{};

    for (const auto& [symbol, length] : symbolLengths) {
        if (length == 0 || length >= lengthCounts.size()) {
            throw std::runtime_error("Invalid Huffman code length generated");
        }

        ++lengthCounts[length];
    }

    return lengthCounts;
}

void limitCodeLengthsToJpegBaseline(std::array<uint16_t, 257>& lengthCounts) {
    for (int length = 256; length > 16; --length) {
        while (lengthCounts[static_cast<std::size_t>(length)] > 0) {
            int shorterLength = length - 2;

            while (shorterLength > 0 &&
                   lengthCounts[static_cast<std::size_t>(shorterLength)] == 0) {
                --shorterLength;
            }

            if (shorterLength <= 0) {
                throw std::runtime_error(
                    "Failed to limit Huffman code lengths to JPEG baseline constraint");
            }

            lengthCounts[static_cast<std::size_t>(length)] -= 2;
            lengthCounts[static_cast<std::size_t>(length - 1)] += 1;

            lengthCounts[static_cast<std::size_t>(shorterLength + 1)] += 2;
            lengthCounts[static_cast<std::size_t>(shorterLength)] -= 1;
        }
    }
}

std::vector<uint8_t> buildSymbolsOrderedByFrequency(
    const HuffmanGenerator::FrequencyTable& frequencies) {
    std::vector<uint8_t> symbols;

    for (std::size_t symbol = 0; symbol < frequencies.size(); ++symbol) {
        if (frequencies[symbol] > 0) {
            symbols.push_back(static_cast<uint8_t>(symbol));
        }
    }

    std::sort(symbols.begin(), symbols.end(),
              [&frequencies](uint8_t a, uint8_t b) {
                  const uint32_t freqA = frequencies[static_cast<std::size_t>(a)];
                  const uint32_t freqB = frequencies[static_cast<std::size_t>(b)];

                  if (freqA != freqB) {
                      return freqA > freqB;
                  }

                  return a < b;
              });

    return symbols;
}

} // namespace

uint8_t HuffmanGenerator::makeAcSymbol(const EncodedACValue& value) {
    if (value.isEob) {
        return 0x00;
    }

    if (value.isZrl) {
        return 0xF0;
    }

    return static_cast<uint8_t>(((value.runLength & 0x0F) << 4) |
                                (value.size & 0x0F));
}

void HuffmanGenerator::countChannelFrequencies(const EntropyEncodedChannel& channel,
                                               FrequencyTable& dcFrequencies,
                                               FrequencyTable& acFrequencies) {
    for (const auto& block : channel.blocks) {
        if (block.dc.category < 0 || block.dc.category > 255) {
            throw std::runtime_error("Invalid DC Huffman category while counting frequencies");
        }

        ++dcFrequencies[static_cast<std::size_t>(block.dc.category)];

        for (const auto& acValue : block.acValues) {
            const uint8_t symbol = makeAcSymbol(acValue);
            ++acFrequencies[static_cast<std::size_t>(symbol)];
        }
    }
}

JpegHuffmanTable HuffmanGenerator::generateFromFrequencies(const FrequencyTable& frequencies) {
    std::vector<uint8_t> symbols;

    for (std::size_t symbol = 0; symbol < frequencies.size(); ++symbol) {
        if (frequencies[symbol] > 0) {
            symbols.push_back(static_cast<uint8_t>(symbol));
        }
    }

    if (symbols.empty()) {
        throw std::runtime_error("Cannot generate Huffman table from empty frequency table");
    }

    std::sort(symbols.begin(), symbols.end(),
              [&frequencies](uint8_t a, uint8_t b) {
                  const uint32_t freqA = frequencies[static_cast<std::size_t>(a)];
                  const uint32_t freqB = frequencies[static_cast<std::size_t>(b)];

                  if (freqA != freqB) {
                      return freqA > freqB;
                  }

                  return a < b;
              });

    std::size_t codeLength = 1;
    std::size_t capacity = 2;

    /*
    * JPEG entropy-coded scan data is padded with 1 bits before byte alignment.
    * To avoid ambiguity, the generated Huffman table must leave at least one
    * unused code word, so no real symbol receives the all-ones code.
    */
    while (capacity <= symbols.size()) {
        ++codeLength;
        capacity <<= 1;
    }

    if (codeLength > 16) {
        throw std::runtime_error(
            "Too many Huffman symbols to encode within JPEG baseline 16-bit limit");
    }

    JpegHuffmanTable table;

    if (symbols.size() > 255) {
        throw std::runtime_error("Generated Huffman table contains too many symbols");
    }

    table.codeLengthCounts[codeLength - 1] = static_cast<uint8_t>(symbols.size());
    table.symbols = std::move(symbols);

    return table;
}

JpegHuffmanTableSet HuffmanGenerator::generateFromEntropyData(const EntropyImageData& entropyData) {
    FrequencyTable luminanceDC{};
    FrequencyTable luminanceAC{};
    FrequencyTable chrominanceDC{};
    FrequencyTable chrominanceAC{};

    countChannelFrequencies(entropyData.y, luminanceDC, luminanceAC);

    countChannelFrequencies(entropyData.cb, chrominanceDC, chrominanceAC);
    countChannelFrequencies(entropyData.cr, chrominanceDC, chrominanceAC);

    return JpegHuffmanTableSet{
        generateFromFrequencies(luminanceDC),
        generateFromFrequencies(luminanceAC),
        generateFromFrequencies(chrominanceDC),
        generateFromFrequencies(chrominanceAC)
    };
}