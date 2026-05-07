#include "bitstream_writer.hpp"

#include <stdexcept>

void BitstreamWriter::writeBit(bool bit) {
    ensureValidState();

    currentByte_ = static_cast<uint8_t>(currentByte_ << 1);

    if (bit) {
        currentByte_ = static_cast<uint8_t>(currentByte_ | 0x01u);
    }

    ++bitsFilled_;

    if (bitsFilled_ == kBitsPerByte) {
        appendCompletedByte(currentByte_);
        currentByte_ = 0;
        bitsFilled_ = 0;
    }
}

void BitstreamWriter::writeBits(uint16_t bits, uint8_t bitCount) {
    ensureValidState();

    if (bitCount > 16) {
        throw std::runtime_error("BitstreamWriter supports writing at most 16 bits at once");
    }

    if (bitCount == 0) {
        return;
    }

    /*
     * JPEG Huffman codes and amplitude bits are written MSB-first.
     * Only the lowest `bitCount` bits are considered.
     */
    for (int bitIndex = static_cast<int>(bitCount) - 1; bitIndex >= 0; --bitIndex) {
        const bool bit = ((bits >> bitIndex) & 0x01u) != 0u;
        writeBit(bit);
    }
}

void BitstreamWriter::writeBits(const std::vector<bool>& bits) {
    ensureValidState();

    for (bool bit : bits) {
        writeBit(bit);
    }
}

void BitstreamWriter::flushWithOnes() {
    ensureValidState();

    if (bitsFilled_ == 0) {
        return;
    }

    const uint8_t remainingBits = static_cast<uint8_t>(kBitsPerByte - bitsFilled_);

    currentByte_ = static_cast<uint8_t>(currentByte_ << remainingBits);

    const uint8_t paddingMask =
        static_cast<uint8_t>((1u << remainingBits) - 1u);

    currentByte_ = static_cast<uint8_t>(currentByte_ | paddingMask);

    appendCompletedByte(currentByte_);

    currentByte_ = 0;
    bitsFilled_ = 0;
}

void BitstreamWriter::reset() {
    buffer_.clear();
    currentByte_ = 0;
    bitsFilled_ = 0;
}

const std::vector<uint8_t>& BitstreamWriter::buffer() const {
    return buffer_;
}

std::size_t BitstreamWriter::size() const {
    return buffer_.size();
}

bool BitstreamWriter::empty() const {
    return buffer_.empty();
}

void BitstreamWriter::appendCompletedByte(uint8_t byte) {
    buffer_.push_back(byte);

    if (byte == 0xFF) {
        buffer_.push_back(kByteStuffingValue);
    }
}

void BitstreamWriter::ensureValidState() const {
    if (bitsFilled_ >= kBitsPerByte) {
        throw std::logic_error("Invalid BitstreamWriter state: bitsFilled_ must be in range [0, 7]");
    }
}