#include "fbmagic/fbmagic.h"

void fbmagic_stroke(fbmagic_ctx* ctx, size_t x, size_t y,
		size_t width, size_t height, size_t stroke_width,
		uint32_t color_val) {
	
	fbmagic_fill(ctx, x, y, width, stroke_width, color_val);
	fbmagic_fill(ctx, x, y, stroke_width, height, color_val);
	fbmagic_fill(ctx, x + (width - stroke_width), y, stroke_width,
			height, color_val);
	fbmagic_fill(ctx, x, y + (height - stroke_width), width,
			stroke_width, color_val);
}
