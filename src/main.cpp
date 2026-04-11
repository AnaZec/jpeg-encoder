#include "bmp_reader.hpp"
#include "color_converter.hpp"
#include "padding.hpp"

#include <iostream>

int main() {
    try {
        BmpImage image = BmpReader::load("../images/input/test.bmp");
        YCbCrImage ycbcr = ColorConverter::rgbToYCbCr(image.data, image.width, image.height);
        YCbCrImage padded = Padding::padToMultipleOf8(ycbcr);

        std::cout << "Original size: " << ycbcr.width << " x " << ycbcr.height << "\n";
        std::cout << "Padded size:   " << padded.width << " x " << padded.height << "\n";

        std::cout << "Width multiple of 8: " << (padded.width % 8 == 0 ? "yes" : "no") << "\n";
        std::cout << "Height multiple of 8: " << (padded.height % 8 == 0 ? "yes" : "no") << "\n";

        std::cout << "Original first Y pixel: " << static_cast<int>(ycbcr.y[0]) << "\n";
        std::cout << "Padded first Y pixel:   " << static_cast<int>(padded.y[0]) << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}