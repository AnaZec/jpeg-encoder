#pragma once

#include <string>

class VisualComparison {
public:
    static void createSideBySideComparison(const std::string& originalPath,
                                           const std::string& compressedPath,
                                           const std::string& outputPath,
                                           int quality);
};