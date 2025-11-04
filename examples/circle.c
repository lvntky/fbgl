#define FBGL_IMPLEMENTATION
#include "fbgl.h"

int main(void)
{
	fbgl_t buffer;
	if (fbgl_init(NULL, &buffer) == -1) {
		fprintf(stdout, "Error: could not open framebuffer device\n");
		return -1;
	}

	fbgl_point_t circ_center = { 960, 540 };

	fbgl_set_bg(&buffer, 0x00FF0000);
	uint32_t i = 0;
	while (true) {
		circ_center.x = 960 + 200 * cos(i * M_PI / 180);
		circ_center.y = 540 + 200 * sin(i * M_PI / 180);

		fbgl_draw_circle_outline(circ_center.x - 240,
					 circ_center.y - 240, 40, 0xFFFFFF,
					 &buffer);
		i = (i + 1) % 360;
		fbgl_draw_circle_filled(480, 540, 40, 0xFFFFFF, &buffer);
		nanosleep((struct timespec[]){ { 0, 10000000 } }, NULL);
	}

	while (1) {
	}

	return 0;
}
