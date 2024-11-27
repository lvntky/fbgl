#define FBGL_IMPLEMENTATION
#include "fbgl.h"

int main()
{
	fbgl_t buffer;
	if (fbgl_init("/dev/fb0", &buffer) == -1) {
		fprintf(stdout, "Error: could not open framebuffer device\n");
		return -1;
	}

	fbgl_set_bg(&buffer, 0x00FF0000);
	fbgl_point_t start = { 0, 0 };
	fbgl_point_t end = { 1020, 1020 };
	for (int i = 0; i < 1890; i++) {
		start.x = i;
		fbgl_draw_line(start, end, 0xFFFFFF, &buffer);
		nanosleep((struct timespec[]){ { 0, 10000000 } }, NULL);
	}
	fbgl_draw_line(start, end, 0x000000, &buffer);

	while (1) {
	}

	return 0;
}
