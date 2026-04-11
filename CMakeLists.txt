#
cmake_minimum_required(VERSION 3.10)
project(jpeg_encoder)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(jpeg_encoder
    src/main.cpp
    src/bmp_reader.cpp
)

target_include_directories(jpeg_encoder PRIVATE inlcude)