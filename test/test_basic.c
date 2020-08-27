#include "fbmagic/fbmagic.h"

int main() {
	fbmagic_image* image;
	fbmagic_font* font;
	fbmagic_ctx* ctx = fbmagic_init("/dev/fb1");
	if (ctx == 0) {
		return 1;
	}

	image = fbmagic_load_bmp("./test/test.bmp");
	if (image == 0) {
		return 1;
	}

	font = fbmagic_load_bdf("./test/bitocra.bdf");
	if (font == 0) {
		return 1;
	}


	fbmagic_fill(ctx, 20, 20, 300, 220, fbmagic_color_value(ctx, 0, 0, 100));

	fbmagic_draw_text(ctx, font, 10, 70, "quick brown fox", fbmagic_color_value(ctx, 255, 255, 255), 3);

	fbmagic_draw_image(ctx, 100, 100, image);

	fbmagic_flush(ctx);

	fbmagic_exit(ctx);

	return 0;
}
