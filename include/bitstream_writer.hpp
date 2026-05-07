#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

/**
 * Writes JPEG entropy-coded bits into a byte buffer.
 *
 * JPEG entropy data is written most-significant-bit first.
 * Whenever a completed byte has value 0xFF, a stuffed 0x00 byte is inserted
 * immediately after it so the entropy stream cannot be confused with JPEG markers.
 */
class BitstreamWriter {
public:

    void writeBit(bool bit);

    void writeBits(uint16_t bits, uint8_t bitCount);

    void writeBits(const std::vector<bool>& bits);

    void flushWithOnes();

    void reset();

    [[nodiscard]] const std::vector<uint8_t>& buffer() const;
    [[nodiscard]] std::size_t size() const;
    [[nodiscard]] bool empty() const;

private:
    static constexpr uint8_t kBitsPerByte = 8;
    static constexpr uint8_t kByteStuffingValue = 0x00;

    void appendCompletedByte(uint8_t byte);
    void ensureValidState() const;

    std::vector<uint8_t> buffer_{};
    uint8_t currentByte_ = 0;
    uint8_t bitsFilled_ = 0;
};