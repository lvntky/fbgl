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
void fbgl_destroy(void);

/**
* Drawing functions
*/
void fbgl_clear(uint32_t color);
void fbgl_put_pixel(int x, int y, uint32_t color);
void fbgl_draw_line(int x0, int y0, int x1, int y1, uint32_t color);

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
	fb->fd = device == NULL ? open(DEFAULT_FB, O_RDWR) :
				  open(device, O_RDWR);
	if (fb->fd == -1) {
		perror("Error openning framebuffer device");
		return -1;
	}
}

#endif

#ifdef __cplusplus
} // extern "C"
#endif
#endif // __FBGL_H__
