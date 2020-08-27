#include "fbmagic/structs.h"
#include "log.h"
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>

fbmagic_ctx* fbmagic_init(const char* device) {
	fbmagic_ctx* ctx = (fbmagic_ctx*)malloc(sizeof(fbmagic_ctx));

	if (ctx == 0) {
		mlog(LOG_ERROR, "Failed to allocate ctx");
		return 0;
	}
	
	ctx->fbfd = open(device, O_RDWR);
	if (ctx->fbfd == -1) {
		mlog(LOG_ERROR, "Cannot open fb device");
		free(ctx);
		return 0;
	}

	if (ioctl(ctx->fbfd, FBIOGET_FSCREENINFO, &ctx->finfo)) {
		mlog(LOG_ERROR, "Failed to get fixed info");
		free(ctx);
		return 0;
	}

	if (ioctl(ctx->fbfd, FBIOGET_VSCREENINFO, &ctx->vinfo)) {
		mlog(LOG_ERROR, "Failed to get variable info");
		free(ctx);
		return 0;
	}

	if (ctx->vinfo.bits_per_pixel > 32 || (ctx->vinfo.bits_per_pixel % 8) != 0) {
		mlog(LOG_ERROR, "BPP must be divisible by 8 and <= 32 bits");
		free(ctx);
		return 0;
	}

	mlog(LOG_DEBUG, "Display info: %dx%d, %d bits per pixel", ctx->vinfo.xres,
			ctx->vinfo.yres, ctx->vinfo.bits_per_pixel);

	

	ctx->buf = mmap(0, ctx->finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED,
			ctx->fbfd, 0);
	if (ctx->buf == 0) {
		mlog(LOG_ERROR, "Failed to mmap");
		free(ctx);
		return 0;
	}

	ctx->vbuf = (char*)malloc(sizeof(char) * ctx->finfo.smem_len);
	if (ctx->vbuf == 0) {
		mlog(LOG_ERROR, "Failed to alloc vbuf");
		free(ctx);
		return 0;
	}

	return ctx;
}

void fbmagic_flush(fbmagic_ctx* ctx) {
	memcpy(ctx->buf, ctx->vbuf, ctx->finfo.smem_len);
}

void fbmagic_exit(fbmagic_ctx* ctx) {
	close(ctx->fbfd);
	munmap(ctx->buf, ctx->finfo.smem_len);
	free(ctx->vbuf);
	free(ctx);
	mlog(LOG_DEBUG, "fbmagic exited");
}
