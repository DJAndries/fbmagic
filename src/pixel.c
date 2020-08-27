#include "fbmagic/fbmagic.h"
#include <string.h>
#include "log.h"

static uint32_t mix_channel(struct fb_bitfield* bitfield, uint32_t curr_val, uint8_t add_val) {
	uint32_t max_val = 0xFFFFFFFF >> (32 - bitfield->length);
	uint32_t adj_val = (uint32_t)(((float)add_val / 0xFF) * max_val);
	return curr_val | (adj_val << bitfield->offset);
}

uint32_t fbmagic_color_value(fbmagic_ctx* ctx, uint8_t r, uint8_t g, uint8_t b) {
	uint32_t result = 0;

	result = mix_channel(&ctx->vinfo.red, result, r);
	result = mix_channel(&ctx->vinfo.green, result, g);
	result = mix_channel(&ctx->vinfo.blue, result, b);
	result = mix_channel(&ctx->vinfo.transp, result, 0xFF);

	return result;
}

void fbmagic_write_pixel(fbmagic_ctx* ctx, size_t x, size_t y, uint32_t color_val) {
	int bytes_per_pixel;
	size_t start_index;

	bytes_per_pixel = ctx->vinfo.bits_per_pixel / 8;
	start_index = (y * ctx->vinfo.xres * bytes_per_pixel) + (x * bytes_per_pixel);

	if (start_index >= ctx->finfo.smem_len) {
		mlog(LOG_ERROR, "Pixel pos not in range: x = %lu y = %lu", x, y);
		return;
	}

	memcpy(ctx->vbuf + start_index, &color_val, bytes_per_pixel);
}
