#define FBGL_IMPLEMENTATION
#include "fbgl.h"

int main(int argc, char *argv[])
{
	fbgl_t buffer;
	if (fbgl_init("/dev/fb0", &buffer) == -1) {
		fprintf(stdout, "Error: could not open framebuffer device\n");
		return -1;
	}

	fprintf(stdout, "Framebuffer width: %d\n", fb_get_width(&buffer));
	fprintf(stdout, "Framebuffer height: %d\n", fb_get_height(&buffer));
	fprintf(stdout, "Framebuffer screen size: %d\n", buffer.screen_size);

	return 0;
}
