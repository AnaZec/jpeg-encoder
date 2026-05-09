# Huffman Encoding in the JPEG Encoder

Huffman encoding is the final compression stage in the JPEG pipeline.

By the time the encoder reaches this stage, the image has already gone through:

```text
RGB to YCbCr
→ 8x8 block splitting
→ DCT
→ quantization
→ zigzag ordering
```

After quantization, each 8x8 block contains 64 integer coefficients.

The first coefficient is the DC coefficient.
The remaining 63 coefficients are AC coefficients.

JPEG does not Huffman-encode these coefficients directly.

Instead, it first prepares them in two different ways:

```text
DC coefficient → differential coding
AC coefficients → run-length encoding
```

Then the prepared symbols are encoded using Huffman tables.

## DC Coefficient Encoding

The DC coefficient represents the average value of an 8x8 block.

Neighboring image blocks often have similar average brightness, so their DC coefficients are usually close to each other.

Instead of storing the full DC value for every block, JPEG stores the difference from the previous block's DC value.

This is called DC differential coding, or DPCM.

For example:

```text
Block DC values:
100, 104, 107, 103
```

JPEG stores:

```text
100, 4, 3, -4
```

The first block is compared against an initial previous DC value of 0.

So the differences are:

```text
100 - 0   = 100
104 - 100 = 4
107 - 104 = 3
103 - 107 = -4
```

This is useful because small differences are cheaper to encode than large absolute values.

## DC Category

JPEG does not directly Huffman-code the difference value itself.
First, it determines the category of the difference.
The category tells how many bits are needed to represent the magnitude of the value.

For example:

```text
Difference value	Category
0	0
-1, 1	1
-3, -2, 2, 3	2
-7 to -4, 4 to 7	3
-15 to -8, 8 to 15	4
-31 to -16, 16 to 31	5
```

So for this DC difference:

```text
diff = 4
```

the category is:

```text
category = 3
```

because 4 needs 3 bits.

The encoder writes:

Huffman code for category 3
+ additional bits representing the value 4

So DC encoding has two parts:

```text
DC symbol:      category
Extra bits:     actual difference magnitude/sign
```

## AC Coefficient Encoding

The AC coefficients describe texture, edges, and fine detail inside the block.

After quantization, many AC coefficients become zero, especially near the end of the zigzag order.

For example, after zigzag ordering, the coefficients may look like:

DC, 5, 0, 0, -3, 0, 0, 0, 2, 0, 0, 0, 0, ...

JPEG uses run-length encoding for AC coefficients.

Instead of storing every zero, it stores:

number of zeros before the next non-zero value
+ size category of the non-zero value
+ additional bits for the value itself

The AC symbol is commonly written as:

RUN/SIZE

Where:

+ RUN is the number of consecutive zeros before the value
+ SIZE is the category of the non-zero value

## AC RLE Example

Consider these AC coefficients after zigzag ordering:

5, 0, 0, -3, 0, 0, 0, 2, 0, 0, 0, 0, ...

The encoder reads them from left to right.

### First value: 5

There are no zeros before it.

RUN = 0
VALUE = 5
SIZE = 3

So the symbol is:

0/3

The encoder writes:

Huffman code for 0/3
+ additional bits for 5

### Second non-zero value: -3

There are two zeros before it.

RUN = 2
VALUE = -3
SIZE = 2

So the symbol is:

2/2

The encoder writes:

Huffman code for 2/2
+ additional bits for -3

### Third non-zero value: 2

There are three zeros before it.

RUN = 3
VALUE = 2
SIZE = 2

So the symbol is:

3/2

The encoder writes:

Huffman code for 3/2
+ additional bits for 2

If the remaining coefficients are all zero, the encoder writes an End-of-Block symbol.

EOB = 0/0

This tells the decoder that the rest of the 8x8 block contains zeros.

## Special AC Symbols

JPEG has two important special AC symbols.

### EOB: End of Block

If all remaining AC coefficients are zero, the encoder writes:

0/0

This is the End-of-Block symbol.

It avoids writing unnecessary zero values until the end of the block.

### ZRL: Zero Run Length

The maximum zero run that can be stored in one AC symbol is 15.

If there are 16 consecutive zeros, JPEG writes a special symbol:

15/0

This is called ZRL, or Zero Run Length.

It means:

16 zeros

For example, if the encoder sees 20 zeros before the next non-zero coefficient, it writes:

ZRL
then RUN = 4 before the next value

## Huffman Tables

Huffman coding assigns shorter bit codes to more common symbols and longer bit codes to less common symbols.

JPEG uses separate Huffman tables for:

DC luminance
AC luminance
DC chrominance
AC chrominance

In this encoder, the table used depends on both:

the coefficient type: DC or AC
the color channel: Y or Cb/Cr

The Y channel stores luminance, which represents brightness.

The Cb and Cr channels store chrominance, which represents color difference.

Because luminance and chrominance data have different statistical behavior, JPEG commonly uses different Huffman tables for them.

## Huffman Table Structure

A JPEG Huffman table describes:

which symbols exist

and

how long their Huffman codes are

The table does not need to store every final code explicitly.

Instead, JPEG stores enough information for the decoder to reconstruct the same canonical Huffman codes.

A JPEG DHT marker contains:

table class
table identifier
number of codes for each code length
symbols ordered by code length

The table class identifies whether the table is for DC or AC coding.

0 = DC table
1 = AC table

The table identifier selects the specific table.

Commonly:

0 = luminance table
1 = chrominance table

So the encoder may write tables such as:

class 0, id 0 → DC luminance
class 1, id 0 → AC luminance
class 0, id 1 → DC chrominance
class 1, id 1 → AC chrominance

## What Actually Gets Written

For each block, the encoder writes entropy-coded bits in this general order:

DC symbol Huffman code
DC additional bits
AC symbol Huffman code
AC additional bits
AC symbol Huffman code
AC additional bits
...
EOB Huffman code

For a Y block, the encoder uses luminance Huffman tables.

For Cb and Cr blocks, it uses chrominance Huffman tables.

## Full Walkthrough

Suppose the previous DC coefficient was:

previous DC = 40

and the current quantized block after zigzag ordering is:

45, 5, 0, 0, -3, 0, 0, 0, 2, 0, 0, 0, ...

The first value is the DC coefficient:

current DC = 45

The DC difference is:

diff = current DC - previous DC
diff = 45 - 40
diff = 5

The category for 5 is:

category = 3

So the encoder writes:

Huffman code for DC category 3
additional bits for value 5

Now the AC coefficients are:

5, 0, 0, -3, 0, 0, 0, 2, 0, 0, 0, ...

The first AC value is 5.

RUN = 0
SIZE = 3
VALUE = 5

The encoder writes:

Huffman code for AC symbol 0/3
additional bits for value 5

Next non-zero AC value is -3.

There are two zeros before it.

RUN = 2
SIZE = 2
VALUE = -3

The encoder writes:

Huffman code for AC symbol 2/2
additional bits for value -3

Next non-zero AC value is 2.

There are three zeros before it.

RUN = 3
SIZE = 2
VALUE = 2

The encoder writes:

Huffman code for AC symbol 3/2
additional bits for value 2

If all remaining AC coefficients are zero, the encoder writes:

Huffman code for EOB

So the symbolic encoded sequence is:

DC: category 3, value bits for 5
AC: 0/3, value bits for 5
AC: 2/2, value bits for -3
AC: 3/2, value bits for 2
AC: EOB

This is much smaller than storing all 64 coefficients directly.

## Why This Compresses Well

This works well because quantization creates many zeros.

After zigzag ordering, those zeros often appear in long runs near the end of the block.

Run-length encoding turns those repeated zeros into compact symbols.

Huffman coding then assigns short bit patterns to common symbols such as:

small DC categories
small AC values
short zero runs
End-of-Block

The result is a compact entropy-coded bitstream.

## Summary

JPEG Huffman encoding is not applied directly to raw pixels or raw DCT coefficients.

The encoder first prepares the quantized coefficients:

DC coefficient
→ difference from previous block DC
→ category
→ Huffman code + additional bits
AC coefficients
→ zigzag order
→ zero run-length + value category
→ Huffman code + additional bits

The Huffman tables define the variable-length codes used for these symbols.

Together, DC differential coding, AC run-length encoding, and Huffman coding produce the final compressed entropy data written into the JPEG file.