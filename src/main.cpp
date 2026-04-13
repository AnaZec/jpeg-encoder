#include "bmp_reader.hpp"
#include "quantizer.hpp"
#include "jpeg_writer.hpp"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    try {
        int qualityFactor = 50;

        if (argc >= 2) {
            qualityFactor = std::stoi(argv[1]);
        }

        const std::string inputPath = "../images/input/test.bmp";
        const std::string outputPath = "../images/output/test.jpg";

        BmpImage image = BmpReader::load(inputPath);

        const auto lumTable = Quantizer::scaledLuminanceTable(qualityFactor);
        const auto chromaTable = Quantizer::scaledChrominanceTable(qualityFactor);

        const std::size_t outputBytes = JpegWriter::writeJpegFile(outputPath,
                                                                  image.width,
                                                                  image.height,
                                                                  lumTable,
                                                                  chromaTable);

        const std::uintmax_t inputBytes = std::filesystem::file_size(inputPath);

        if (outputBytes == 0) {
            throw std::runtime_error("Output JPEG file size is zero");
        }

        const double compressionRatio =
            static_cast<double>(inputBytes) / static_cast<double>(outputBytes);

        std::cout << "JPEG file written successfully: " << outputPath << "\n";
        std::cout << "Quality factor: " << qualityFactor << "\n";
        std::cout << "Input size: " << inputBytes << " bytes\n";
        std::cout << "Output size: " << outputBytes << " bytes\n";
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Compression ratio: " << compressionRatio << ":1\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}