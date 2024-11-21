#ifndef __FBGL_H__
#define __FBGL_H__

#define VERSION "0.1.0"
#define NAME "FBGL"
#define DEFAULT_FB "/dev/fb0"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/fb.h>
#include <stddef.h>
#include <stdio.h>

/**
* Structs
*/
typedef struct fbgl {
	int width;
	int height;
	int fd;
	size_t screen_size;
	uint32_t *pixels;
	struct fb_var_screeninfo vinfo; // Variable screen information
	struct fb_fix_screeninfo finfo; // Fixed screen information
} fbgl_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * General purpose methods
 */
char const *fbgl_name_info(void);
char const *fbgl_version_info(void);

/*Create and destroy methods*/
int fbgl_init(const char *device, fbgl_t *fb);
void fbgl_destroy(fbgl_t *fb);

/**
* Drawing functions
*/
void fbgl_clear(uint32_t color);
void fbgl_put_pixel(int x, int y, uint32_t color);
void fbgl_draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void fbgl_set_bg();

/**
* Display methods
*/
void fbgl_display();

/**
* Access framebuffer data methods
*/
uint32_t *fb_get_data(void);
int fb_get_width(void);
int fb_get_height(void);

#ifdef FBGL_IMPLEMENTATION

char const *fbgl_name_info(void)
{
	return NAME;
}

char const *fbgl_version_info(void)
{
	return VERSION;
}

int fbgl_init(const char *device, fbgl_t *fb)
{
	if (!fb) {
		fprintf(stderr, "Error: fbgl_t pointer is NULL.");
		return -1;
	}

	fb->fd = device == NULL ? open(DEFAULT_FB, O_RDWR) :
				  open(device, O_RDWR);
	if (fb->fd == -1) {
		perror("Error openning framebuffer device");
		return -1;
	}

	if (ioctl(fb->fd, FBIOGET_FSCREENINFO, &fb->finfo) == -1) {
		perror("Error: Reading fixed information."); close(fb->fd);
		return -1;
	}
	if (ioctl(fb->fd, FBIOGET_VSCREENINFO, &fb->vinfo) == -1) {
		perror("Error reading variable information");
		close(fb->fd);
		return -1;
	}

	fb->width = fb->vinfo.xres;
	fb->height = fb->vinfo.yres;
	fb->screen_size = fb->finfo.smem_len;

	// Map framebuffer to memory
	fb->pixels = (uint32_t *)mmap(NULL, fb->screen_size,
				      PROT_READ | PROT_WRITE, MAP_SHARED,
				      fb->fd, 0);
	if (fb->pixels == MAP_FAILED) {
		perror("Error mapping framebuffer device to memory");
		close(fb->fd);
		return -1;
	}

	return 0;
}

void fbgl_destroy(fbgl_t *fb)
{
	if (!fb || fb->fd == -1) {
		fprintf(stderr,
			"Error: framebuffer not initialized or already destroyed.\n");
		return;
	}

	if (fb->pixels && fb->pixels != MAP_FAILED) {
		munmap(fb->pixels, fb->screen_size);
	}

	close(fb->fd);
	fb->fd = -1;
}

void fbgl_set_bg(fbgl_t *fb, uint32_t color)
{
	if (!fb || fb->fd == -1) {
		fprintf(stderr, "Error: framebuffer not initialized.\n");
		return;
	}

	// Fill the entire framebuffer with the specified color
	for (size_t i = 0; i < (fb->width * fb->height); ++i) {
		fb->pixels[i] = color;
	}
}

#endif

#ifdef __cplusplus
} // extern "C"
#endif
#endif // __FBGL_H__
