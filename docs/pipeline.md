# Encoder Pipeline Architecture

This document describes the modular structure of the JPEG encoder pipeline.

## Overview

The application entry point is intentionally kept small. `main.cpp` is responsible for:

- parsing command-line arguments
- discovering BMP input files
- creating output directories
- configuring logging
- invoking the encoder pipeline for each image
- returning batch-level exit codes

The actual image-processing flow is handled by `EncoderPipeline`.

## Pipeline Stages

The encoder pipeline is composed of the following stages:

```text
BMP input
  ↓
BmpReader
  ↓
ColorConverter
  ↓
Padding
  ↓
BlockSplitter
  ↓
DCT
  ↓
Quantizer
  ↓
ZigZag
  ↓
EntropyEncoder
  ↓
JpegWriter
  ↓
Metrics + VisualComparison
```

| Stage    | Component          | Responsibility                                              |
| -------- | ------------------ | ----------------------------------------------------------- |
| Load     | `BmpReader`        | Load and validate 24-bit BMP input                          |
| Convert  | `ColorConverter`   | Convert RGB pixels to YCbCr                                 |
| Pad      | `Padding`          | Pad dimensions to multiples of 8                            |
| Block    | `BlockSplitter`    | Split channels into 8x8 blocks                              |
| DCT      | `DCT`              | Transform spatial blocks into frequency coefficients        |
| Quantize | `Quantizer`        | Apply quality-scaled quantization tables                    |
| Reorder  | `ZigZag`           | Reorder coefficients from low to high frequency             |
| Encode   | `EntropyEncoder`   | Prepare DC/AC entropy symbols                               |
| Write    | `JpegWriter`       | Write JPEG markers, tables, entropy stream, and output file |
| Validate | `Metrics`          | Compute MSE and PSNR using decoded output                   |
| Compare  | `VisualComparison` | Generate side-by-side visual comparison images              |
