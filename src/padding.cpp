#include "padding.hpp"
#include <algorithm>
#include <stdexcept>
#include <vector>

namespace {
    int roundUpToMultipleOf8(int value){
        return ((value + 7) / 8) * 8;
    }


std::vector<uint8_t> padChannel(const std::vector<uint8_t>& input, int width, int height, int paddedWidth, int paddedHeight){

    if(width <= 0 || height <= 0){
        throw std::runtime_error("Invalid channel dimensions for padding");
    }

    if(static_cast<int>(input.size()) != width * height){
        throw std::runtime_error("Channel size does not match dimensions");
    }

    std::vector<uint8_t> output(static_cast<std::size_t>(paddedWidth) * paddedHeight);

    for(int y = 0; y < paddedHeight; ++y){
        const int srcY = std::min(y,height-1);

        for(int x = 0; x < paddedWidth; ++x){
            const int srcX = std::min(x, width-1);

            output[static_cast<std::size_t>(y) * paddedWidth + x] = input[static_cast<std::size_t>(srcY) * width + srcX];
        }
    }

    return output;
}
} // namespace

YCbCrImage Padding::padToMultipleOf8(const YCbCrImage& input){
    if(input.width <= 0 || input.height <= 0){
        throw std::runtime_error("Invalid image dimensions for padding");
    }

    const int paddedHeight = roundUpToMultipleOf8(input.height);
    const int paddedWidth = roundUpToMultipleOf8(input.width);

    YCbCrImage output;
    output.width = paddedWidth;
    output.height = paddedHeight;
    output.y = padChannel(input.y, input.width, input.height, paddedWidth, paddedHeight);
    output.cb = padChannel(input.cb, input.width, input.height, paddedWidth, paddedHeight);
    output.cr = padChannel(input.cr, input.width, input.height, paddedWidth, paddedHeight);

    return output;
}