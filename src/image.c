#include "fbmagic/fbmagic.h"
#include <stdio.h>
#include <stdlib.h>
#include "log.h"

static int seek_and_read(FILE* file, long int offset, size_t element_size, size_t n_element, void* dest) {
	size_t result;
	if (fseek(file, offset, SEEK_SET)) {
		return 1;
	}
	result = fread(dest, element_size, n_element, file);
	if (result < 1) {
		return 1;
	}
	return 0;
}

static fbmagic_image* parse_header(FILE* file) {
	fbmagic_image* image;
	uint16_t bpp;
	uint32_t start_addr, width, height, compression;
	char buf[16];

	if (fread(buf, sizeof(char), 2, file) < 2 || buf[0] != 0x42 || buf[1] != 0x4D) {
		mlog(LOG_ERROR, "BMP: bad magic number");
		return 0;
	}

	if (seek_and_read(file, 0xA, sizeof(uint32_t), 1, &start_addr)) {
		mlog(LOG_ERROR, "BMP: fail to read start addr");
		return 0;	
	}

	if (seek_and_read(file, 0x12, sizeof(uint32_t), 1, &width)) {
		mlog(LOG_ERROR, "BMP: fail to read width");
		return 0;
	}

	if (seek_and_read(file, 0x16, sizeof(uint32_t), 1, &height)) {
		mlog(LOG_ERROR, "BMP: fail to read height");
		return 0;
	}

	if (seek_and_read(file, 0x1C, sizeof(uint16_t), 1, &bpp)) {
		mlog(LOG_ERROR, "BMP: fail to read bpp");
		return 0;
	}
	if (bpp != 24 && bpp != 32) {
		mlog(LOG_ERROR, "Only 24 bit and 32 bit BMPs are supported");
		return 0;
	}

	if (seek_and_read(file, 0x1E, sizeof(uint32_t), 1, &compression)) {
		mlog(LOG_ERROR, "BMP: fail to read compression method");
		return 0;
	}
	if (compression != 0x00 && compression != 0x03) {
		mlog(LOG_ERROR, "BMP has compression, no compression supported");
		return 0;
	}

	if ((image = (fbmagic_image*)malloc(sizeof(fbmagic_image))) == 0) {
		mlog(LOG_ERROR, "Failed to alloc image container");
		return 0;
	}

	if ((image->data = (char*)malloc(sizeof(char) * (bpp / 8) * width * height)) == 0) {
		mlog(LOG_ERROR, "Failed to alloc image data");
		free(image);
		return 0;
	}

	image->width = width;
	image->height = height;
	image->bpp = bpp;
	image->bmp_start_addr = start_addr;

	return image;
}

fbmagic_image* fbmagic_load_bmp(const char* filename) {
	FILE* file;
	fbmagic_image* image;

	file = fopen(filename, "rb");
	if (file == 0) {
		mlog(LOG_ERROR, "Failed to open bmp");
		return 0;
	}

	if ((image = parse_header(file)) == 0) {
		fclose(file);
		return 0;
	}

	if (seek_and_read(file, image->bmp_start_addr, sizeof(char),
				image->height * image->width * (image->bpp / 8), image->data)) {
		mlog(LOG_ERROR, "Failed to read img data");
		free(image->data);
		free(image);
		fclose(file);
		return 0;
	}

	fclose(file);
	return image;
}

void fbmagic_draw_image(fbmagic_ctx* ctx, size_t x, size_t y, fbmagic_image* image, float scale) {
	size_t data_i, ix, iy, sx, sy;
	uint32_t color_val;
	int bytes_per_pixel = image->bpp / 8;

	for (iy = 0; iy < (size_t)(image->height * scale); iy++) {
		for (ix = 0; ix < (size_t)(image->width * scale); ix++) {
			sy = (size_t)(iy / scale);
			sx = (size_t)(ix / scale);
			data_i = ((image->width * (image->height - sy - 1))
					+ sx) * bytes_per_pixel;
			if (bytes_per_pixel == 4) {
				if (image->data[data_i + 3] < 128) {
					continue;
				}
			}
			color_val = fbmagic_color_value(ctx, image->data[data_i + 2],
					image->data[data_i + 1], image->data[data_i]);
			fbmagic_write_pixel(ctx, x + ix, y + iy, color_val);
		}
	}
}

void fbmagic_free_image(fbmagic_image* image) {
	free(image->data);
	free(image);
}
