# Performance Notes

This document records performance observations and optimization work for the JPEG encoder.

## DCT Block Processing Loop Optimization

The main block-processing bottleneck was identified in the forward DCT stage.

The original implementation recomputed cosine values inside the innermost loop of the 8x8 DCT calculation. Since JPEG uses fixed 8x8 blocks, these cosine values are constant and can be precomputed once and reused for every block.

## Change

The DCT implementation was refactored to precompute:

- 8x8 cosine lookup values
- DCT normalization scale factors

This removes repeated calls to `std::cos()` from the innermost block-processing loop.

## Benchmark Setup

Test image:

```text
images/input/test1.bmp
```

| Quality | DCT Time | Entropy Encoding | JPEG Writing | Total Runtime | Output Size |
|---:|---:|---:|---:|---:|---:|
| Q=75 | 12,413 ms | 2,122 ms | 818 ms | 16.861 s | 2,389,237 bytes |
| Q=5 | 12,546 ms | 1,017 ms | 469 ms | 15.555 s | 1,004,477 bytes |

```
