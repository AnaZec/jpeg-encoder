#include "bmp_reader.hpp"
#include "quantizer.hpp"
#include "jpeg_writer.hpp"

#include <filesystem>
#include <iostream>
#include <string>

int main() {
    try {
        BmpImage image = BmpReader::load("../images/input/test.bmp");

        const auto lumTable = Quantizer::scaledLuminanceTable(50);
        const auto chromaTable = Quantizer::scaledChrominanceTable(50);

        const std::string outputPath = "../images/output/test.jpg";

        JpegWriter::writeJpegFile(outputPath,
                                  image.width,
                                  image.height,
                                  lumTable,
                                  chromaTable);

        std::cout << "JPEG file written successfully: " << outputPath << "\n";
        std::cout << "File size: " << std::filesystem::file_size(outputPath) << " bytes\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}