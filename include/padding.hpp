#pragma once
#include "color_converter.hpp"

class Padding{
    public:
        static YCbCrImage padToMultipleOf8(const YCbCrImage& input);
};