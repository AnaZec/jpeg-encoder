#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

class BitstreamWriter {
public:
    void writeBit(bool bit);
    void writeBits(uint16_t bits, uint8_t bitCount);
    void writeBits(const std::vector<bool>& bits);

    void flushWithOnes();
    void reset();

    const std::vector<uint8_t>& buffer() const;
    std::size_t size() const;
    bool empty() const;

private:
    void pushByte(uint8_t byte);

    std::vector<uint8_t> buffer_{};
    uint8_t currentByte_ = 0;
    uint8_t bitsFilled_ = 0;
};