# fbmagic

Simple C library for drawing to a framebuffer device

## Operations

- Fill single pixel
- Fill a rectangle of pixels
- Stroke/draw line
- Load/draw a simple BMP image (only 24-bit and 32-bit uncompressed BMPs supported)
  - Draw function which includes alpha blending
  - Another draw function which performs better, but has no alpha blending
- Load/draw BDF font/text
- Device locking; good for multiple processes that require access to the device

## Build

Run `make`.

## Usage

See `include/fbmagic/fbmagic.h` for available functions.
