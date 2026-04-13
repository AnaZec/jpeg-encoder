#include "bitstream_writer.hpp"

#include <stdexcept>

void BitstreamWriter::writeBit(bool bit) {
    currentByte_ <<= 1;
    if (bit) {
        currentByte_ |= 0x01;
    }

    ++bitsFilled_;

    if (bitsFilled_ == 8) {
        pushByte(currentByte_);
        currentByte_ = 0;
        bitsFilled_ = 0;
    }
}

void BitstreamWriter::writeBits(uint16_t bits, uint8_t bitCount) {
    if (bitCount > 16) {
        throw std::runtime_error("BitstreamWriter only supports writing up to 16 bits at once");
    }

    if (bitCount == 0) {
        return;
    }

    for (int i = bitCount - 1; i >= 0; --i) {
        writeBit(((bits >> i) & 0x01u) != 0u);
    }
}

void BitstreamWriter::writeBits(const std::vector<bool>& bits) {
    for (bool bit : bits) {
        writeBit(bit);
    }
}

void BitstreamWriter::flushWithOnes() {
    if (bitsFilled_ == 0) {
        return;
    }

    while (bitsFilled_ != 0) {
        writeBit(true);
    }
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

void BitstreamWriter::pushByte(uint8_t byte) {
    buffer_.push_back(byte);

    if (byte == 0xFF) {
        buffer_.push_back(0x00);
    }
}