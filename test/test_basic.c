#include "fbmagic/fbmagic.h"

int main() {
	fbmagic_image* image;
	fbmagic_ctx* ctx = fbmagic_init("/dev/fb1");
	if (ctx == 0) {
		return 1;
	}

	image = fbmagic_load_bmp("./test/test.bmp");
	if (image == 0) {
		return 1;
	}

	fbmagic_fill(ctx, 20, 20, 300, 220, fbmagic_color_value(ctx, 0, 0, 100));

	fbmagic_draw_image(ctx, 100, 100, image);

	fbmagic_flush(ctx);

	fbmagic_exit(ctx);

	return 0;
}
