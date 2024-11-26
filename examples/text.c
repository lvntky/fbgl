#define FBGL_IMPLEMENTATION
#include "fbgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for usleep
#include <stdint.h>

int main(int argc, char *argv[])
{
	fbgl_t fb;
	fbgl_init("/dev/fb0", &fb);

	fbgl_set_bg(&fb, 0xFFFFFF);

	fbgl_psf1_font_t *font = fbgl_load_psf1_font(argv[1]);
	fbgl_render_psf1_text(&fb, font, "hello, fbgl", 100, 100, 0x000000);
	size_t framerate = 30 * 30;

	for(size_t i = 0; i < framerate; i++) {
	    usleep(50000);
	}
	fbgl_destroy_psf1_font(font);
	fbgl_destroy(&fb);

	return 0;
}
