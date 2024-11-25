#define FBGL_IMPLEMENTATION
#include "fbgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for usleep
#include <stdint.h>

int main(int argc, char *argv[])
{
    fbgl_t buf;
    fbgl_init("/dev/fb0", &buf);

    fbgl_set_bg(&buf, 0x000000);

    fbgl_psf2_font_t *font = fbgl_load_psf2_font(argv[1]);

    size_t framerate = 30 * 30;
    fbgl_render_psf2_text(&buf, font, "hello fbgl", 100, 100, 0xFFFFFF);

    for(size_t i = 0; i < framerate; i++) {
	usleep(50000);
    }

    fbgl_destroy_psf2_font(font);
    fbgl_destroy(&buf);
    
    return 0;
}
