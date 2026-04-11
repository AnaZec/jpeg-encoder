#include "bmp_reader.hpp"

#include <iostream>

int main() {
    try {
        BmpImage image = BmpReader::load("../images/input/test.bmp");

        std::cout << "Loaded BMP successfully\n";
        std::cout << "Width: " << image.width << "\n";
        std::cout << "Height: " << image.height << "\n";
        std::cout << "Pixel bytes: " << image.data.size() << "\n";

        if (!image.data.empty()) {
            std::cout << "First pixel RGB = ("
                      << static_cast<int>(image.data[0]) << ", "
                      << static_cast<int>(image.data[1]) << ", "
                      << static_cast<int>(image.data[2]) << ")\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}