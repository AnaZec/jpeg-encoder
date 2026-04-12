#include "bmp_reader.hpp"
#include "quantizer.hpp"
#include "jpeg_writer.hpp"

#include <filesystem>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    try {
        int qualityFactor = 50;

        if (argc >= 2) {
            qualityFactor = std::stoi(argv[1]);
        }

        BmpImage image = BmpReader::load("../images/input/test.bmp");

        const auto lumTable = Quantizer::scaledLuminanceTable(qualityFactor);
        const auto chromaTable = Quantizer::scaledChrominanceTable(qualityFactor);

        const std::string outputPath = "../images/output/test.jpg";

        JpegWriter::writeJpegFile(outputPath,
                                  image.width,
                                  image.height,
                                  lumTable,
                                  chromaTable);

        std::cout << "JPEG file written successfully: " << outputPath << "\n";
        std::cout << "Quality factor: " << qualityFactor << "\n";
        std::cout << "File size: " << std::filesystem::file_size(outputPath) << " bytes\n";

        std::cout << "First 8 luminance table values: ";
        for (int i = 0; i < 8; ++i) {
            std::cout << lumTable[i] << " ";
        }
        std::cout << "\n";

        std::cout << "First 8 chrominance table values: ";
        for (int i = 0; i < 8; ++i) {
            std::cout << chromaTable[i] << " ";
        }
        std::cout << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}