#ifndef FBMAGIC_STRUCTS_H
#define FBMAGIC_STRUCTS_H

#include <linux/fb.h>
#include <stddef.h>

struct fbmagic_ctx {
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
	int fbfd;
	char* buf;
	char* vbuf;
};
typedef struct fbmagic_ctx fbmagic_ctx;

struct fbmagic_image {
	char* data;
	size_t width;
	size_t height;
	size_t bpp;

	size_t bmp_start_addr;
};
typedef struct fbmagic_image fbmagic_image;

#endif
