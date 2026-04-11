#include "bmp_reader.hpp"
#include "color_converter.hpp"

#include <iostream>

int main() {
    try {
        BmpImage image = BmpReader::load("../images/input/test.bmp");
        YCbCrImage ycbcr = ColorConverter::rgbToYCbCr(image.data, image.width, image.height);

        std::cout << "Loaded BMP successfully\n";
        std::cout << "Width: " << image.width << "\n";
        std::cout << "Height: " << image.height << "\n";
        std::cout << "Converted RGB to YCbCr successfully\n";

        if (!image.data.empty()) {
            std::cout << "First RGB pixel = ("
                      << static_cast<int>(image.data[0]) << ", "
                      << static_cast<int>(image.data[1]) << ", "
                      << static_cast<int>(image.data[2]) << ")\n";

            std::cout << "First YCbCr pixel = ("
                      << static_cast<int>(ycbcr.y[0]) << ", "
                      << static_cast<int>(ycbcr.cb[0]) << ", "
                      << static_cast<int>(ycbcr.cr[0]) << ")\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}