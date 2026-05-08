# Memory Handling Notes

This document describes memory handling improvements for large input images.

## Issue #34: Large Image Memory Handling

The encoder processes images through several full-image intermediate representations:

```text
BMP RGB buffer
  ↓
YCbCr image
  ↓
padded YCbCr image
  ↓
8x8 block buffers
  ↓
DCT coefficient blocks
  ↓
quantized blocks
  ↓
zigzag-ordered blocks
  ↓
entropy encoded data
  ↓
JPEG bitstream
```

Earlier versions kept most intermediate buffers alive until the end of the image-processing cycle. This increased peak memory usage for large images.

## Change

The pipeline now releases large intermediate objects as soon as they are no longer needed.

## Examples

* BMP RGB data is released after RGB to YCbCr conversion
* unpadded YCbCr data is released after padding
* 8x8 byte blocks and padded YCbCr buffers are released after DCT
* DCT coefficient blocks are released after quantization
* quantized blocks are released after zigzag ordering
* zigzag blocks are released after entropy encoding
* entropy data is released after JPEG writing

This reduces peak memory usage without changing the encoder output.

## Large Image Validation

The encoder was validated on:
```
images/input/test1.bmp
```
Image dimensions: 2560x3840

This is approximately: 9.8 megapixels

The acceptance criterion requires images larger than 4MP. This test image is more than twice that size.

## Memory Measurement

Peak memory can be measured with:

```bash
/usr/bin/time -v ./jpeg_encoder
/usr/bin/time -v ./jpeg_encoder --quality 5
```
The most relevant field is: Maximum resident set size

## Measurement Results


| Quality | Command                      | Maximum Resident Set Size | Approx. Peak Memory | Exit Status |
| ------: | ---------------------------- | ------------------------: | ------------------: | ----------: |
|    Q=75 | `./jpeg_encoder`             |                412,484 KB |             ~403 MB |           0 |
|     Q=5 | `./jpeg_encoder --quality 5` |                387,212 KB |             ~378 MB |           0 |

```