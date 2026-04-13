cmake_minimum_required(VERSION 3.10)
project(jpeg_encoder)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

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
    src/huffman_tables.cpp
    src/entropy_encoder.cpp
    src/bitstream_writer.cpp
    src/jpeg_writer.cpp
    src/metrics.cpp
    src/debug_dump.cpp
)

target_include_directories(jpeg_encoder PRIVATE
    include
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(jpeg_encoder PRIVATE ${OpenCV_LIBS})