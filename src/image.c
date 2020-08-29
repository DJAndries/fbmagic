#include "fbmagic/fbmagic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

	image->width = width;
	image->height = height;
	image->bpp = bpp;
	image->bmp_start_addr = start_addr;

	return image;
}

static int convert_data(fbmagic_ctx* ctx, FILE* file, fbmagic_image* image) {
	char* tdata;
	size_t x, y, data_i;
	uint32_t color_value;
	unsigned int dbytes_per_pixel = ctx->vinfo.bits_per_pixel / 8;
	unsigned int sbytes_per_pixel = image->bpp / 8;

	if ((tdata = (char*)malloc(sizeof(char) * sbytes_per_pixel * image->width * image->height)) == 0) {
		mlog(LOG_ERROR, "Failed to alloc image temp data");
		return 1;
	}
	if ((image->data = (char*)malloc(sizeof(char) * dbytes_per_pixel * image->width * image->height)) == 0) {
		mlog(LOG_ERROR, "Failed to alloc image data");
		free(tdata);
		return 1;
	}
	if ((image->alpha_map = (unsigned char*)malloc(sizeof(char) * image->width * image->height)) == 0) {
		mlog(LOG_ERROR, "Failed to alloc alpha map");
		free(image->data);
		free(tdata);
		return 1;
	}
	if (seek_and_read(file, image->bmp_start_addr, sizeof(char),
				image->height * image->width * sbytes_per_pixel, tdata)) {
		mlog(LOG_ERROR, "Failed to read img data");
		free(tdata);
		free(image->data);
		free(image->alpha_map);
		return 2;
	}

	for (y = 0; y < image->height; y++) {
		for (x = 0; x < image->width; x++) {
			data_i = ((image->height - y - 1) * image->width + x) * sbytes_per_pixel;

			image->alpha_map[y * image->width + x] = sbytes_per_pixel == 4 ?
				tdata[data_i + 3] : 255;

			color_value = fbmagic_color_value(ctx, tdata[data_i + 2],
					tdata[data_i + 1], tdata[data_i]);

			memcpy(image->data + ((y * image->width + x) * dbytes_per_pixel),
					&color_value, dbytes_per_pixel);
		}
	}

	free(tdata);
	return 0;
}

fbmagic_image* fbmagic_load_bmp(fbmagic_ctx* ctx, const char* filename) {
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

	if (convert_data(ctx, file, image)) {
		free(image);
		fclose(file);
		return 0;
	}


	fclose(file);
	return image;
}

void fbmagic_draw_image(fbmagic_ctx* ctx, size_t x, size_t y, fbmagic_image* image, float scale) {
	size_t ix, iy, sx, sy, si, sheight, swidth;
	uint32_t color_val;
	unsigned int bytes_per_pixel = ctx->vinfo.bits_per_pixel / 8;

	sheight = (size_t)(image->height * scale);
	swidth = (size_t)(image->width * scale);

	for (iy = 0; iy < sheight; iy++) {
		for (ix = 0; ix < swidth; ix++) {
			sy = (size_t)(iy / scale);
			sx = (size_t)(ix / scale);
			si = sy * image->width + sx;

			if (image->alpha_map[si] < 128) {
				continue;
			}

			memcpy(&color_val, image->data + (si * bytes_per_pixel),
					bytes_per_pixel);

			fbmagic_write_pixel(ctx, x + ix, y + iy, color_val);
		}
	}
}

void fbmagic_draw_image_quick(fbmagic_ctx* ctx, size_t x, size_t y, fbmagic_image* image) {
	size_t iy, si, di, img_width_bytes, screen_width_bytes;
	unsigned int bytes_per_pixel = ctx->vinfo.bits_per_pixel / 8;

	img_width_bytes = image->width * bytes_per_pixel;
	screen_width_bytes = ctx->vinfo.xres * bytes_per_pixel;

	di = (y * screen_width_bytes) + (x * bytes_per_pixel);
	si = 0;
	for (iy = 0; iy < image->height; iy++) {
			memcpy(ctx->vbuf + di, image->data + si, img_width_bytes);
			
			di += screen_width_bytes;
			si += img_width_bytes;
	}
}

void fbmagic_free_image(fbmagic_image* image) {
	free(image->data);
	free(image);
}
