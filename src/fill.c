#include "fbmagic/fbmagic.h"

void fbmagic_fill(fbmagic_ctx* ctx, size_t x, size_t y, size_t width, size_t height, uint32_t color_val) {
	size_t ix, iy;

	for (iy = y; iy < height; iy++) {
		for (ix = x; ix < width; ix++) {
			fbmagic_write_pixel(ctx, ix, iy, color_val);
		}
	}
}

