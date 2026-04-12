cmake_minimum_required(VERSION 3.10)
project(jpeg_encoder)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(jpeg_encoder
    src/main.cpp
    src/bmp_reader.cpp
    src/color_converter.cpp
    src/padding.cpp
    src/block_splitter.cpp
    src/dct.cpp
    src/quantizer.cpp
    src/zigzag.cpp
    src/dc_encoder.cpp
    src/ac_encoder.cpp
    src/jpeg_writer.cpp
    src/dct_test.cpp
    src/zigzag_test.cpp
)

target_include_directories(jpeg_encoder PRIVATE include)