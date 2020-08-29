#ifndef FBMAGIC_STRUCTS_H
#define FBMAGIC_STRUCTS_H

#include <linux/fb.h>
#include <stddef.h>
#include <stdint.h>

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
	unsigned char* alpha_map;
	size_t width;
	size_t height;
	size_t bpp;

	size_t bmp_start_addr;
};
typedef struct fbmagic_image fbmagic_image;

struct fbmagic_font_char {
	unsigned short width;
	unsigned short height;
	short x;
	short y;
	uint32_t data[32];
};
typedef struct fbmagic_font_char fbmagic_font_char;

struct fbmagic_font {
	fbmagic_font_char chars[256];
};
typedef struct fbmagic_font fbmagic_font;

#endif
