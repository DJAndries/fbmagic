#ifndef FBMAGIC_FBMAGIC_H
#define FBMAGIC_FBMAGIC_H

#include <stddef.h>
#include <stdint.h>
#include "structs.h"

fbmagic_ctx* fbmagic_init(const char* device);

void fbmagic_flush(fbmagic_ctx* ctx);

void fbmagic_exit(fbmagic_ctx* ctx);

uint32_t fbmagic_color_value(fbmagic_ctx* ctx, uint8_t r, uint8_t g, uint8_t b);

void fbmagic_write_pixel(fbmagic_ctx* ctx, size_t x, size_t y, uint32_t color_val);

void fbmagic_fill(fbmagic_ctx* ctx, size_t x, size_t y, size_t width, size_t height,
		uint32_t color_val);

fbmagic_image* fbmagic_load_bmp(const char* filename);

void fbmagic_draw_image(fbmagic_ctx* ctx, size_t x, size_t y, fbmagic_image* image);

#endif
