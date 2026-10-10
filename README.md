# Steganography

A command-line tool written in C++ that hides text messages inside image files and extracts them again, using a password to decide where in the image the data goes.

## How it works

The message is converted to bits, and each bit replaces the least significant bit of the **red channel** of one pixel. Changing the lowest bit shifts that channel's value by at most 1, which is invisible to the eye.

What makes this more than a textbook LSB implementation is **where** the bits go. Instead of writing them into consecutive pixels, the password is hashed (FNV-1a) and the result seeds a Mersenne Twister (`std::mt19937_64`). That generator produces the sequence of pixel positions used for the message. The same password reproduces the same sequence, so only someone with the password knows which pixels to read, and the modified pixels are scattered across the whole image instead of forming a detectable block in the top-left corner.

Two details keep this reliable:

- **Every position is used at most once.** A position that was already drawn is skipped, so one bit can never overwrite another, and a message can fill the image to its full capacity of one bit per pixel.
- **The sequence is identical on every platform.** The hash is a fixed algorithm rather than `std::hash`, and positions come straight from the generator's output rather than from `std::uniform_int_distribution`; both of those are implementation-defined, so an image encoded with one compiler could otherwise fail to decode with another.

## Requirements

- C++20 compiler (uses `std::format`)
- [SFML](https://www.sfml-dev.org/) – image loading and pixel access
- [fmt](https://github.com/fmtlib/fmt) – console output
- CMake

Tested on Windows 10/11.

## Build

```bash
git clone https://github.com/GustawP/Steganography.git
cd Steganography
cmake -B build
cmake --build build
```

## Usage

```
-i, --info     <file path>                 Show file information
-e, --encrypt  <file path>  "<message>"    Hide a message in the file
-d, --decrypt  <file path>  <length>       Recover a message from the file
-c, --check    <file path>  "<message>"    Check whether the message fits
-h, --help                                 Show help
```

The password is not passed as an argument: the program prompts for it after launch, so it does not end up in shell history.

### Examples

```bash
# Check capacity first
steganography -c photo.bmp "Secret message"

# Hide the message (prompts for a password)
steganography -e photo.bmp "Secret message"

# Read it back - the length in characters is required
steganography -d photo.bmp 14
```

Supported formats: **BMP** and **PNG**. Both are lossless, which matters, because any recompression would destroy the hidden bits.

Capacity is one bit per pixel, so a 400×300 image holds up to 15,000 bytes. `-e` refuses a message that does not fit, and `-d` refuses a length larger than the image could hold.

## Known limitations

Documented rather than hidden, since they are the interesting part of the project:

- **The message length must be known to decode.** There is no length header or terminator stored in the image, so `-d` takes the character count as an argument.
- **The original file is overwritten.** `-e` saves back to the input path, so the unmodified image is lost unless it was copied beforehand.
- **Non-ASCII characters.** Multi-byte UTF-8 characters occupy more than one byte, so the length passed to `-d` has to be the byte count rather than the visible character count.
- **Images encoded with the 2024 version cannot be decoded by this one.** Fixing position collisions and making the sequence portable changed which pixels a password selects.

## What I learned

- Pixel-level image manipulation and bit masking (`color.r & 0xFE`)
- Using a hashed password to seed a PRNG so that a pseudo-random sequence becomes reproducible for anyone holding the key
- That "reproducible" has to hold across compilers too: `std::hash` and the standard distributions are implementation-defined, so portable output needs fixed algorithms
- Why sampling positions independently is not the same as choosing distinct positions, and how a collision silently corrupts data instead of failing loudly
- Why lossless formats are a hard requirement for this kind of embedding
- Designing a CLI with separate verbs for inspecting, checking and modifying a file

## Status

University project from 2024, revisited in 2026 to fix position collisions, make encoding portable across compilers and validate input.
