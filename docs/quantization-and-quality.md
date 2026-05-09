# Quantization and JPEG Quality

Quantization is the main lossy step in baseline JPEG compression.

Before quantization, the encoder has already converted each `8x8` image block from pixel values into DCT coefficients. These coefficients describe the block as frequency information:

- low-frequency coefficients describe smooth brightness and color changes
- high-frequency coefficients describe fine details, sharp edges, and texture

The DCT itself does not remove information. It only changes how the block is represented.

Quantization is the step that reduces precision.

This is where JPEG decides which details are important enough to keep and which details can be stored with less accuracy or removed.

## Why Quantization Works

Natural images usually contain many smooth areas, such as:

- sky
- walls
- roads
- shadows
- skin
- blurred backgrounds

In these areas, most visually important information is concentrated in low-frequency DCT coefficients.

High-frequency coefficients are often smaller and less visually important. They may represent fine texture, small color variations, sensor noise, or sharp edges.

JPEG takes advantage of this by preserving low-frequency information more carefully and reducing high-frequency information more aggressively.

## Quantization Table

JPEG uses a quantization table to decide how much precision to remove from each DCT coefficient.

Each DCT coefficient is divided by a corresponding value from the quantization table.

Conceptually:

```text
quantized coefficient = round(DCT coefficient / quantization table value)
```

A small table value keeps more precision.

A large table value removes more precision.

For example:

```text
DCT coefficient:          120
Quantization value:        10
Quantized coefficient:     12
```

But if the quantization value is larger:

```text
DCT coefficient:          120
Quantization value:        40
Quantized coefficient:      3
```

The second result stores the coefficient with much less precision.
When the image is decoded, the value is multiplied back by the same quantization value:
```text
reconstructed coefficient = quantized coefficient * quantization table value
```

However, the original precision is already lost.

For example:
```text
Original DCT coefficient:       118
Quantization value:              16
Quantized coefficient: round(118 / 16) = 7
Reconstructed coefficient: 7 * 16 = 112
```
The reconstructed value is close, but not identical. The difference is the quantization error.

## Example: Before and After Quantization

Suppose an 8x8 block produces these DCT coefficients:

```text
620  -42   18    7    3    1    0    0
-36   24  -12    5    2    1    0    0
 20  -10    6   -3    1    0    0    0
  8    4   -2    1    0    0    0    0
  3   -1    1    0    0    0    0    0
  1    0    0    0    0    0    0    0
  0    0    0    0    0    0    0    0
  0    0    0    0    0    0    0    0
```

A simplified quantization table may look like this:

```text
16  11  10  16  24  40  51  61
12  12  14  19  26  58  60  55
14  13  16  24  40  57  69  56
14  17  22  29  51  87  80  62
18  22  37  56  68 109 103  77
24  35  55  64  81 104 113  92
49  64  78  87 103 121 120 101
72  92  95  98 112 100 103  99
```

After dividing and rounding, the quantized block may become:

```text
39  -4   2   0   0   0   0   0
-3   2  -1   0   0   0   0   0
 1  -1   0   0   0   0   0   0
 1   0   0   0   0   0   0   0
 0   0   0   0   0   0   0   0
 0   0   0   0   0   0   0   0
 0   0   0   0   0   0   0   0
 0   0   0   0   0   0   0   0
```

Many small high-frequency coefficients became zero.

This is useful because the next JPEG stages can compress zeros very efficiently.

After quantization, JPEG applies zigzag ordering, which scans the block from low frequencies toward high frequencies. Since many high-frequency coefficients are now zero, run-length encoding can represent long zero sequences compactly.

## Why File Size Gets Smaller

Quantization reduces file size because it creates many repeated zeros.
Before quantization, the DCT block may contain many small non-zero values.
After quantization, many of those values become zero.
That helps the entropy coding stage because it can encode patterns like:

```text
several zeros followed by one non-zero value
```

instead of storing every coefficient individually.

Stronger quantization usually means:

- more coefficients become zero
- entropy coding becomes more efficient
- file size decreases
- image quality decreases

Weaker quantization usually means:

- fewer coefficients become zero
- more detail is preserved
- file size increases
- image quality improves

## Quality Factor

This encoder supports a user-provided quality factor.
The quality factor controls how strongly the quantization tables are scaled.
A higher quality value keeps more detail.
A lower quality value removes more detail.

Conceptually:

higher quality factor
- smaller effective quantization values
- less aggressive division
- more non-zero coefficients
- larger file size
- better visual quality

lower quality factor
- larger effective quantization values
- more aggressive division
- more zero coefficients
- smaller file size
- lower visual quality

For example:

| Quality setting | Quantization strength | Expected file size |    Expected visual quality |
| --------------- | --------------------: | -----------------: | -------------------------: |
| Low quality     |   Strong quantization |            Smaller |             More artifacts |
| Medium quality  | Balanced quantization |             Medium |         Acceptable quality |
| High quality    |     Weak quantization |             Larger | Better detail preservation |

## What Low Quality Looks Like

At low quality settings, JPEG uses stronger quantization.

This can cause visible artifacts such as:

blockiness around 8x8 block boundaries
loss of fine texture
ringing near sharp edges
blurred details
color banding or color smearing

These artifacts happen because high-frequency information was reduced too aggressively.

## What High Quality Looks Like

At high quality settings, JPEG uses weaker quantization.

More DCT coefficients remain non-zero, especially in detailed regions.

This preserves:

- sharper edges
- more texture
- smoother gradients
- better local detail

The tradeoff is that the output JPEG file becomes larger.

## Why Quantization Affects High Frequencies More

The quantization table usually contains smaller values near the top-left and larger values toward the bottom-right.

This matches the DCT coefficient layout:

```text
top-left      ->  low frequencies
bottom-right  ->  high frequencies
```

Low frequencies are usually more visually important, so JPEG keeps them more accurately.
High frequencies are often less visually important, so JPEG stores them with less precision.
This is why the quantization table is not flat. It is designed around human visual perception.

## Summary

Quantization is the main lossy compression step in JPEG.

It works by dividing each DCT coefficient by a value from a quantization table and rounding the result.

This reduces precision, especially for high-frequency coefficients.

The result is that many small coefficients become zero, making the later zigzag, run-length, and Huffman coding stages much more effective.

The quality factor controls how aggressive this process is:

* higher quality means weaker quantization, better image quality, and larger files
* lower quality means stronger quantization, lower image quality, and smaller files

In short:

```text
DCT separates image information by frequency.
Quantization decides how much of that information to keep.
```

## Related Reading

Quantization creates many zero-valued coefficients, especially in the high-frequency part of each block.

The next entropy-coding stage uses zigzag ordering, run-length encoding, and Huffman coding to store those zeros efficiently.

See [Huffman Encoding in the JPEG Encoder](huffman-encoding.md).