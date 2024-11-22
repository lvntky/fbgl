#define FBGL_IMPLEMENTATION
#include "../fbgl.h"

int main(int argc, char *argv[])
{
	fbgl_t buffer;
	if (fbgl_init("/dev/fb0", &buffer) == -1) {
		fprintf(stdout, "Error: could not open framebuffer device\n");
		return -1;
	}

	fbgl_set_bg(&buffer, 0xFF0000);
	fbgl_point_t start = { 0, 0 };
	fbgl_point_t end = { 1020, 1020};
	for(int i = 0; i < 1890; i++) {
		start.x = i;
		fbgl_draw_line(start, end, 0xFFFFFF, &buffer);
		for(int j = 0; j < 10000000; j++){}
	}
	fbgl_draw_line(start, end, 0x000000, &buffer);

	while (1) {
	}

	return 0;
}
