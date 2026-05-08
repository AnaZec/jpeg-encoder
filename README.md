# JPEG Encoder in C++17

A baseline JPEG encoder implemented in C++17.

This project converts uncompressed BMP images into JPEG files by manually implementing the main stages of the JPEG compression pipeline: BMP loading, RGB to YCbCr conversion, 8x8 block processing, DCT, quantization, zigzag ordering, entropy preparation, Huffman coding, bitstream writing, JPEG marker serialization, metrics, and visual comparison output.

The goal of this project is to demonstrate low-level C++ image-processing implementation, binary file serialization, data-structure design, memory handling, and compression-pipeline understanding.

## Project Highlights

- Baseline JPEG-style encoder implemented in C++17
- Manual BMP loading and validation
- RGB to YCbCr color conversion
- Padding to 8x8-compatible dimensions
- 8x8 block splitting
- Forward DCT implementation
- Quality-factor-based quantization
- Zigzag coefficient ordering
- DC differential encoding
- AC run-length encoding
- Dynamic Huffman table generation from entropy-symbol frequencies
- Bitstream writer with JPEG byte stuffing
- JPEG marker writing: SOI, APP0, DQT, SOF0, DHT, SOS, EOI
- MSE and PSNR validation using OpenCV
- Side-by-side original/compressed comparison output
- Stage-level logging and runtime reporting
- Memory handling improvements for large images
- Performance optimizations for DCT and entropy encoding

## What This Project Demonstrates

This project is not a wrapper around an existing JPEG encoder. The main compression stages are implemented manually.

OpenCV is used only for:

- loading the generated JPEG back for validation
- computing image-quality metrics
- generating visual comparison images

The encoder itself handles the JPEG pipeline, coefficient processing, entropy preparation, Huffman coding, bitstream generation, and JPEG file writing.

## Repository Structure

```text
jpeg-encoder/
├── include/                 # Public headers
├── src/                     # C++ implementation files
├── images/
│   ├── input/               # BMP input images
│   ├── output/              # Generated JPEG outputs
│   └── comparisons/         # Side-by-side comparison images
├── reports/                 # Generated per-run metric reports
├── docs/                    # Architecture, performance, memory, and artifact notes
├── CMakeLists.txt
└── README.md
```

## Encoding Pipeline 

```text
BMP input
  ↓
BMP loading and validation
  ↓
RGB to YCbCr conversion
  ↓
Padding to multiples of 8
  ↓
8x8 block splitting
  ↓
Forward DCT
  ↓
Quantization
  ↓
Zigzag ordering
  ↓
DC/AC entropy preparation
  ↓
Huffman coding
  ↓
Bitstream writing
  ↓
JPEG marker serialization
  ↓
JPEG output
```

## Pipeline Stages Explained 

1. BMP Loading

The encoder starts from a 24-bit uncompressed BMP image.

The BMP reader validates the file header, checks the image dimensions, rejects unsupported BMP variants, and loads pixel data into an internal RGB buffer.

Supported input format:

24-bit uncompressed BMP

Unsupported or malformed BMP files are rejected with clear error messages.

2. RGB to YCbCr Conversion

JPEG compression is usually performed in the YCbCr color space rather than directly in RGB.

The encoder converts each RGB pixel into:

Y  - luminance / brightness
Cb - blue-difference chroma
Cr - red-difference chroma

This separates brightness information from color information. Human vision is more sensitive to luminance detail than chroma detail, which is one of the reasons JPEG uses this representation.

3. Padding to Multiples of 8

JPEG processes image data in 8x8 blocks.

If the image width or height is not divisible by 8, the encoder pads the image so each channel can be split cleanly into 8x8 blocks.

original dimensions
  ↓
padded dimensions divisible by 8

The original image dimensions are still written into the JPEG metadata so the final decoded image has the correct size.

4. 8x8 Block Splitting

Each Y, Cb, and Cr channel is split into independent 8x8 blocks.

This is the core block-based structure of JPEG compression.

Y channel  → Y blocks
Cb channel → Cb blocks
Cr channel → Cr blocks

Each block is processed independently through DCT, quantization, zigzag ordering, and entropy encoding.

5. Forward DCT

The Discrete Cosine Transform converts each 8x8 block from the spatial domain into the frequency domain.

In simple terms:

```text
pixel values
  ↓
frequency coefficients
```

The top-left coefficient represents the average intensity of the block. Other coefficients represent increasing horizontal and vertical frequency detail.

Low-frequency coefficients describe broad image structure. High-frequency coefficients describe edges, texture, and fine detail.

The DCT implementation uses precomputed cosine and scale-factor tables to avoid redundant calculations inside the hot block-processing loop.

6. Quantization

Quantization is the main lossy step in JPEG compression.

Each DCT coefficient is divided by a value from a quantization table and rounded:

quantized coefficient = round(DCT coefficient / quantization value)

Lower quality factors use stronger quantization. This produces smaller files but more visible artifacts.

Higher quality factors preserve more coefficients. This improves visual quality but increases output size.

The encoder supports quality values in the range:

1 to 100

Default quality: 75

7. Zigzag Ordering

After quantization, each 8x8 block is reordered using the JPEG zigzag scan pattern.

Zigzag ordering places low-frequency coefficients first and high-frequency coefficients later.

This is useful because many high-frequency coefficients become zero after quantization. Grouping them toward the end makes run-length encoding more effective.

8. DC and AC Entropy Preparation

JPEG encodes the first coefficient of each block differently from the remaining coefficients.

The first coefficient is the DC coefficient. It is encoded using the difference from the previous block's DC value.

DC difference = current DC - previous DC

The remaining 63 coefficients are AC coefficients. They are encoded using run-length encoding, where long runs of zeros are represented compactly.

Special AC symbols include:

EOB - End of Block
ZRL - Zero Run Length
9. Huffman Coding

The encoder generates image-specific Huffman tables from entropy-symbol frequencies.

Separate Huffman tables are used for:

luminance DC
luminance AC
chrominance DC
chrominance AC

The generated Huffman tables are written into the JPEG DHT marker, and the scan data is encoded using the matching canonical Huffman codes.

This keeps the JPEG header and entropy-coded bitstream consistent.

10. Bitstream Writing

JPEG entropy data is written bit by bit.

The bitstream writer handles:

most-significant-bit-first writing
byte alignment
final padding with 1 bits
JPEG byte stuffing after 0xFF

Byte stuffing is required because 0xFF is used as the marker prefix in JPEG files. If a real entropy byte has value 0xFF, the encoder writes an extra 0x00 byte after it so decoders do not confuse it with a marker.

11. JPEG File Writing

The JPEG writer serializes the final JPEG structure.

The generated file includes:

SOI  - Start of Image
APP0 - JFIF metadata
DQT  - Quantization tables
SOF0 - Baseline frame header
DHT  - Huffman tables
SOS  - Start of Scan
EOI  - End of Image

The output is a baseline JPEG file with three components:

Y, Cb, Cr
12. Metrics and Visual Comparison

After writing the JPEG file, the encoder reloads the output image with OpenCV and computes:

MSE  - Mean Squared Error
PSNR - Peak Signal-to-Noise Ratio

It also generates side-by-side visual comparison images showing the original BMP next to the compressed JPEG output.

## Build Instructions

### Requirements
C++17 compiler
CMake
OpenCV

On Ubuntu-based systems, dependencies can be installed with:

```bash
sudo apt update
sudo apt install build-essential cmake libopencv-dev
```

### Build
```bash
rm -rf build
mkdir build
cd build
cmake ..
make -j$(nproc)
```

## Usage

Run with the default quality factor, Q=75:

```bash
./jpeg_encoder
```

Run with a custom quality factor:

```bash
./jpeg_encoder --quality 50
```

Run with debug logging:

```bash
./jpeg_encoder --log-level debug
```
Run with error-only logging:

```bash
./jpeg_encoder --log-level error
```

Show help:

```bash
./jpeg_encoder --help
```

## Input and Output

Input BMP images are read from:

```text
images/input/
```

Generated JPEG files are saved to:

```text
images/output/
```

Per-image metric reports are saved to:

```text
reports/
```

Side-by-side visual comparisons are saved to:

```text
images/comparisons/
```

## Example Quality Levels

The encoder can be run at different quality factors to observe the tradeoff between file size and visual quality:

```bash
./jpeg_encoder --quality 5
./jpeg_encoder --quality 50
./jpeg_encoder
./jpeg_encoder --quality 90
```

Lower quality factors produce smaller files but stronger artifacts. Higher quality factors preserve more detail but produce larger files.

## Documentation

Additional project notes are available under docs/:

docs/pipeline.md            - Pipeline architecture and stage responsibilities
docs/performance.md         - Performance optimization notes
docs/memory.md              - Memory handling notes for large images
docs/artifact-analysis.md   - Compression artifact analysis

## Performance Notes

Several performance improvements were made during development.

### DCT Optimization

The original DCT implementation recomputed cosine values inside the innermost loop.

The optimized version precomputes:

cosine lookup values
DCT scale factors

This significantly reduces DCT runtime while preserving the same mathematical formula.

### Entropy Encoding Optimization

The entropy encoder originally stored amplitude bits as std::vector<bool> for each DC and AC value.

This was replaced with packed uint16_t amplitude fields, using the DC category or AC size as the bit count.

This avoids repeated construction of small bit vectors and improves entropy encoding performance.

### Memory Handling

The encoder releases large intermediate buffers as soon as they are no longer needed.

Examples:

BMP RGB data is released after RGB to YCbCr conversion
unpadded YCbCr data is released after padding
8x8 blocks and padded buffers are released after DCT
DCT blocks are released after quantization
quantized blocks are released after zigzag ordering
zigzag blocks are released after entropy encoding
entropy data is released after JPEG writing

This reduces peak memory usage for large images while preserving output behavior.

## Limitations

This project implements a baseline educational JPEG encoder and is not intended to replace production JPEG libraries.

Current limitations:

BMP input only
24-bit uncompressed BMP input only
baseline sequential JPEG only
no progressive JPEG support
no chroma subsampling support yet
no grayscale-only JPEG path yet
no restart markers
no streaming/chunked encoding architecture yet
no SIMD/GPU acceleration yet

## Future Improvements

Possible future work:

* Add chroma subsampling modes such as 4:2:2 and 4:2:0
* Add grayscale-only JPEG encoding path
* Add restart marker support
* Add streaming or chunked block processing
* Add unit tests for individual pipeline stages
* Compare standard Huffman tables against generated Huffman tables
* Add SIMD or multithreaded DCT optimization
* Add more detailed compression-ratio reports

## Why This Project Matters

JPEG encoding combines several important low-level software concepts:

* image representation
* color-space conversion
* block-based signal processing
* frequency-domain transforms
* lossy compression
* entropy coding
* binary file formats
* bit-level writing
* memory management
* performance profiling

This project was built to understand how image data is transformed, compressed, serialized, and validated at a low level in C++.