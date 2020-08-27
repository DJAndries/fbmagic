#include "fbmagic/fbmagic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"

#define LINE_MAX_CHARS 256

static fbmagic_font* free_close_fail(FILE* file, fbmagic_font* font) {
	fclose(file);
	free(font);
	return 0;
}

fbmagic_font* fbmagic_load_bdf(const char* filename) {
	FILE* file;
	fbmagic_font* font;
	char linebuf[LINE_MAX_CHARS];
	char cmd[LINE_MAX_CHARS];
	short curr_bitmap_row = -1;

	unsigned short def_width = 0;
	unsigned short def_height = 0;
	short def_x = 0;
	short def_y = 0;

	unsigned char encoding = 0;
	unsigned short curr_width = 0;
	unsigned short curr_height = 0;
	short curr_x = 0;
	short curr_y = 0;

	file = fopen(filename, "r");
	if (file == 0) {
		mlog(LOG_ERROR, "Failed to open bdf");
		return 0;
	}

	font = (fbmagic_font*)malloc(sizeof(fbmagic_font));
	if (font == 0) {
		mlog(LOG_ERROR, "Unable to allocate font container");
		fclose(file);
		return 0;
	}
	memset(font, 0, sizeof(fbmagic_font));

	while (fgets(linebuf, LINE_MAX_CHARS, file) != 0) {
		if (curr_bitmap_row >= 0) {
			if (sscanf(linebuf, "%x", &font->chars[encoding].data[curr_bitmap_row]) != 1) {
				mlog(LOG_ERROR, "Could not parse bitmap row");
				return free_close_fail(file, font);
			}
			
			if (++curr_bitmap_row == curr_height) {
				curr_bitmap_row = -1;
			}
			continue;
		}
		if (sscanf(linebuf, "%s", cmd) == 0) {
			continue;
		}
		
		if (strcmp("FONTBOUNDINGBOX", cmd) == 0) {
			if (sscanf(linebuf, "%s %hu %hu %hd %hd", cmd, &def_width, &def_height, &def_x, &def_y) != 5) {
				mlog(LOG_ERROR, "bdf font: FONTBOUNDINGBOX is bad");
				return free_close_fail(file, font);
			}
		} else if (strcmp("ENCODING", cmd) == 0) {
			if (sscanf(linebuf, "%s %hhu", cmd, &encoding) != 2) {
				mlog(LOG_ERROR, "bdf font: ENCODING is bad");
				return free_close_fail(file, font);
			}
		} else if (strcmp("STARTCHAR", cmd) == 0) {
			curr_width = def_width;
			curr_height = def_height;
			curr_x = def_x;
			curr_y = def_y;
		} else if (strcmp("BBX", cmd) == 0) {
			if (sscanf(linebuf, "%s %hu %hu %hd %hd", cmd, &curr_width, &curr_height, &curr_x, &curr_y) != 5) {
				mlog(LOG_ERROR, "bdf font: BBX is bad");
				return free_close_fail(file, font);
			}
		} else if (strcmp("BITMAP", cmd) == 0) {
			if (encoding == 0) {
				mlog(LOG_ERROR, "Started BITMAP before ENCODING");
				return free_close_fail(file, font);
			}
			font->chars[encoding].x = curr_x;
			font->chars[encoding].y = curr_y;
			font->chars[encoding].width = curr_width;
			font->chars[encoding].height = curr_height;
			if (curr_height > 0) curr_bitmap_row = 0;
		}
	}

	fclose(file);
	return font;
}

void fbmagic_draw_text(fbmagic_ctx* ctx, fbmagic_font* font, size_t x, size_t y, char* const text, uint32_t color_val, unsigned short scale) {
	fbmagic_font_char* curr_char;
	size_t i, ix, iy, sx, sy;
	size_t starting_bit;
	size_t strlength = strlen(text);
	int curr_x = x;
	unsigned short max_char_height = 0;

	for (i = 0; i < strlength; i++) {
		curr_char = &font->chars[(unsigned)text[i]];

		if (curr_char->height > max_char_height) max_char_height = curr_char->height;
	}
	for (i = 0; i < strlength; i++) {
		if (text[i] == ' ') {
			curr_x += 8;
			continue;
		}
		curr_char = &font->chars[(unsigned)text[i]];

		if (curr_char->width == 0) {
			continue;
		}

		starting_bit = (curr_char->width / 8 * 8) + ((curr_char->width % 8 != 0) * 8);
		
		for (iy = 0; iy < curr_char->height * scale; iy++) {
			for (ix = 0; ix < curr_char->width * scale; ix++) {
				sx = ix / scale;
				sy = iy / scale;
				if (((curr_char->data[sy] >> (starting_bit - sx - 1)) & 0x01) == 0x00) {
					continue;
				}
				fbmagic_write_pixel(ctx, curr_x + ix, 
						y + iy - (curr_char->y * scale) +
						((max_char_height - curr_char->height) * scale), color_val);
			}
		}

		curr_x += (curr_char->width * scale) + 3;
	}
}
