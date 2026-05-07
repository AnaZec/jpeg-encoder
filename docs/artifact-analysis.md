````md
# JPEG Compression Artifact Analysis

This document summarizes visible compression artifacts observed in the generated JPEG outputs at different quality factors. The goal is to connect visual degradation with the encoder stages that cause it, especially quantization, 8x8 block processing, and entropy coding.

The analysis is based on side-by-side comparison outputs generated under:

```bash
./jpeg_encoder --quality 5
./jpeg_encoder --quality 50
./jpeg_encoder --quality 75
./jpeg_encoder --quality 90
````

The generated comparison images are saved under:

```text
images/comparisons/
```

The generated metric reports are saved under:

```text
reports/
```

## Overview

This encoder implements a baseline JPEG-style compression pipeline:

```text
BMP input
  ↓
RGB to YCbCr conversion
  ↓
Padding to dimensions divisible by 8
  ↓
8x8 block splitting
  ↓
DCT
  ↓
Quantization
  ↓
Zigzag ordering
  ↓
DC/AC entropy preparation
  ↓
Huffman coding
  ↓
JPEG bitstream writing
```

Most visible compression artifacts are introduced during the quantization stage. Quantization reduces the precision of DCT coefficients, especially higher-frequency coefficients that represent fine detail, texture, and sharp transitions. Lower quality factors use stronger quantization, which produces smaller outputs but more visible degradation.

The most important artifacts observed are:

* Blocking artifacts
* Loss of fine detail
* Texture smoothing
* Rougher edges
* Reduced sharpness
* Loss of subtle gradients

## Why Blocking Artifacts Appear

JPEG processes the image in independent 8x8 blocks. Each block is transformed, quantized, and reconstructed separately. At low quality factors, aggressive quantization removes a large amount of high-frequency information inside each block.

Because neighboring blocks are processed independently, their reconstructed pixel values may no longer align smoothly at block boundaries. This creates visible square-shaped boundaries known as blocking artifacts.

Blocking is most visible in:

* Smooth regions
* Gradients
* Low-texture areas
* Areas with subtle color transitions
* Regions around edges when quantization is very aggressive

## Why Detail Loss Appears

The DCT separates each 8x8 block into frequency components.

Low-frequency coefficients describe broad image structure, while high-frequency coefficients describe edges, texture, and fine detail. During quantization, high-frequency coefficients are more likely to be reduced heavily or rounded to zero.

As a result:

* Fine textures become smoother
* Small patterns disappear
* Edges become less crisp
* Local contrast is reduced
* The image may look flatter or softer

This is expected behavior for lossy JPEG compression.

## Quality-Level Observations

### Q=5: Severe Compression Artifacts

At quality factor 5, compression artifacts are strongly visible.

Observed artifacts:

* Strong 8x8 blocking artifacts
* Clear discontinuities between neighboring blocks
* Severe loss of fine detail
* Strong smoothing of textures
* Rougher edges
* Loss of subtle gradients
* Flattened regions where detail should be present

Cause:

At Q=5, quantization is very aggressive. Many medium- and high-frequency DCT coefficients are reduced to zero. Since each block loses detail independently, boundaries between 8x8 blocks become much easier to see.

Impact:

Q=5 is useful as a stress test. It clearly exposes the visual weaknesses of aggressive block-based compression. The output is much smaller, but perceptual quality is significantly reduced.

### Q=50: Moderate Compression Artifacts

At quality factor 50, the image remains structurally correct and recognizable, but compression loss is still visible.

Observed artifacts:

* Mild to moderate blocking in smooth areas
* Noticeable loss of fine texture
* Reduced sharpness
* Some small details are simplified or removed
* Edges are mostly preserved but less crisp than the original
* Overall image quality is acceptable but visibly compressed

Cause:

Q=50 uses less aggressive quantization than Q=5, but still removes a meaningful amount of high-frequency information. The main structure of the image remains intact, while finer details are sacrificed.

Impact:

Q=50 is useful for analyzing the tradeoff between file size and visible quality loss. It shows compression artifacts clearly enough to evaluate the encoder, but the result is still visually usable.

### Q=75: Mild Compression Artifacts

Quality factor 75 is the default quality level used by the encoder.

Observed artifacts:

* Overall structure is well preserved
* Blocking artifacts are minimal at normal viewing scale
* Fine details are slightly softened
* Edges remain mostly sharp
* Visual difference from the original is moderate to low
* Image quality is significantly better than Q=50

Cause:

Q=75 preserves more DCT coefficients than lower quality settings. Some high-frequency information is still removed, but not aggressively enough to cause severe visible artifacts.

Impact:

Q=75 provides a practical balance between visual quality and compression. It is a good default setting for demonstrating the encoder because the output remains visually close to the original while still showing measurable compression loss through MSE and PSNR.

### Q=90: Minimal Visible Artifacts

At quality factor 90, the compressed output is visually close to the original.

Observed artifacts:

* Blocking artifacts are very low or not noticeable at normal scale
* Fine detail is mostly preserved
* Edges remain sharp
* Texture loss is minor
* Differences are easier to observe through metrics or close inspection than through casual viewing

Cause:

Q=90 uses weak quantization compared to lower quality factors. More DCT coefficients are preserved, so the reconstructed image retains more high-frequency detail.

Impact:

Q=90 is useful as a high-quality reference point. It demonstrates that the encoder can preserve visual detail when compression is less aggressive, although this comes at the cost of larger output size.

## Artifact Summary Table

| Quality Factor | Blocking Artifacts | Detail Loss | Edge Quality     | Overall Visual Quality |
| -------------: | ------------------ | ----------- | ---------------- | ---------------------- |
|            Q=5 | Severe             | Severe      | Rough / degraded | Low                    |
|           Q=50 | Mild to moderate   | Moderate    | Mostly preserved | Medium                 |
|           Q=75 | Minimal            | Mild        | Good             | Good                   |
|           Q=90 | Very low           | Very low    | Very good        | High                   |

## Connection to Encoder Stages

### 8x8 Block Splitting

The image is divided into independent 8x8 blocks before the DCT stage. This block-based structure is the reason blocking artifacts can appear.

When quantization is strong, neighboring blocks may reconstruct with slightly different boundary values, creating visible square patterns.

Relevant stage:

```text
YCbCr image
  ↓
8x8 block splitting
```

### DCT

The DCT converts each 8x8 block from the spatial domain into the frequency domain.

Low-frequency coefficients represent broad image structure. High-frequency coefficients represent fine details, texture, and sharp transitions.

Relevant stage:

```text
8x8 blocks
  ↓
DCT coefficients
```

When high-frequency coefficients are removed or reduced, the reconstructed image loses detail and becomes smoother.

### Quantization

Quantization is the main source of lossy compression in this encoder.

During quantization, DCT coefficients are divided by values from the quantization table and rounded. Lower quality factors produce larger effective quantization values, causing more coefficients to become zero.

Relevant stage:

```text
DCT coefficients
  ↓
quantized coefficients
```

This reduces file size but introduces visible artifacts.

### Zigzag Ordering

Zigzag ordering does not directly introduce visual artifacts. Its purpose is to arrange coefficients from low frequency to high frequency before entropy encoding.

Relevant stage:

```text
quantized 8x8 block
  ↓
zigzag coefficient sequence
```

This improves compression efficiency because higher-frequency coefficients are often zero after quantization.

### Entropy Coding and Huffman Tables

Entropy coding reduces the encoded file size but does not directly change reconstructed image quality.

The visual quality is already determined by the quantized coefficients before Huffman coding. Dynamic Huffman tables can improve compression efficiency, but they should not change MSE, PSNR, or visual artifacts if the quantized data is the same.

Relevant stage:

```text
DC/AC symbols
  ↓
Huffman coding
  ↓
JPEG bitstream
```

## Findings for Future Optimization

The artifact analysis suggests several useful directions for future work.

### 1. Optimize DCT Runtime

Runtime reports show that DCT dominates the total execution time. The current implementation prioritizes clarity and correctness, but future optimization could improve performance significantly.

Possible improvements:

* Precompute cosine values
* Use separable 1D DCT passes
* Reduce repeated calculations
* Use SIMD instructions
* Add multithreading
* Explore GPU/OpenCL acceleration

### 2. Add Zoomed Artifact Crops

Blocking artifacts and detail loss are easier to inspect in zoomed regions. A future tool could automatically generate cropped comparison images for selected regions.

Possible output:

```text
images/comparisons/crops/
```

This would make artifact analysis easier and more precise.

### 3. Compare Standard vs Generated Huffman Tables

Dynamic Huffman tables affect compressed file size, not visual quality. A useful future experiment would compare output sizes between:

* Standard JPEG Huffman tables
* Generated image-specific Huffman tables

This would help evaluate the compression benefit of dynamic Huffman generation.

### 4. Add Compression Ratio to Reports

The current reports include output size and quality metrics. Future reports could also include compression ratio by comparing the original BMP size with the compressed JPEG size.

Useful metric:

```text
compression ratio = original size / compressed size
```

This would make the quality-size tradeoff clearer.

### 5. Explore Chroma Subsampling

The current encoder processes full-resolution color channels. Future work could add chroma subsampling, such as 4:2:2 or 4:2:0, to better match common JPEG compression pipelines.

This would allow analysis of color-detail tradeoffs and additional compression gains.

## Conclusion

The generated outputs show expected JPEG-style compression behavior. At very low quality factors, blocking artifacts and detail loss are clearly visible due to aggressive quantization of independently processed 8x8 blocks. At medium quality, the main image structure is preserved, but fine detail and sharpness are reduced. At higher quality factors, visible artifacts are minimal and the compressed output remains close to the original.

This confirms that the encoder behaves consistently with a baseline block-based JPEG compression pipeline. The analysis also identifies useful directions for future optimization, especially DCT performance, artifact visualization, compression-ratio reporting, and comparison of standard versus generated Huffman tables.

```