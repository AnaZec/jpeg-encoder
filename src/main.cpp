#include "bmp_reader.hpp"
#include "color_converter.hpp"
#include "padding.hpp"
#include "block_splitter.hpp"
#include "dct.hpp"
#include "quantizer.hpp"
#include "zigzag.hpp"
#include "dc_encoder.hpp"
#include "ac_encoder.hpp"
#include "jpeg_writer.hpp"

#include <iostream>

int main() {
    try {
        BmpImage image = BmpReader::load("../images/input/test.bmp");

        auto lumTable = Quantizer::scaledLuminanceTable(50);
        auto chromaTable = Quantizer::scaledChrominanceTable(50);

        JpegWriter::writeJpegSkeleton("../images/output/test_skeleton.jpg",
                                      image.width,
                                      image.height,
                                      lumTable,
                                      chromaTable);

        std::cout << "JPEG skeleton written successfully.\n";
        std::cout << "Output: ../images/output/test_skeleton.jpg\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}