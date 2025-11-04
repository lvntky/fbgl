#define FBGL_IMPLEMENTATION
#include "fbgl.h"

int main(void)
{
	fbgl_t buffer;
	if (fbgl_init(NULL, &buffer) == -1) {
		fprintf(stdout, "Error: could not open framebuffer device\n");
		return -1;
	}

	fbgl_set_bg(&buffer, 0xFFFFFF); // Set the background to white

	fbgl_point_t start = { 100, 100 };
	fbgl_point_t end = { 200, 200 };
	fbgl_draw_rectangle_outline(start, end, 0xFF0000,
				    &buffer); // Draw red rectangle outline

	fbgl_point_t start2 = { 600, 400 };
	fbgl_point_t end2 = { 800, 800 };
	uint32_t colors[] = { 0xFFC00, 0x00FF00, 0x0000FF,
			      0xFF00FF }; // Yellow, Green, Blue, Magenta
	size_t color_index = 0;

	// Initial position of the marquee rectangle
	int dx = 15; // Horizontal speed
	int dy = 8; // Vertical speed

	while (1) {
		// Clear the framebuffer (set background)
		fbgl_set_bg(&buffer, 0xFFFFFF);

		// Draw the moving filled rectangle
		fbgl_draw_rectangle_filled(start2, end2, colors[color_index],
					   &buffer);

		// Move the filled rectangle by updating its position
		start2.x += dx;
		end2.x += dx;
		start2.y += dy;
		end2.y += dy;

		// Reverse direction if the rectangle hits the screen boundary
		if (start2.x <= 0 || end2.x >= buffer.width) {
			dx = -dx;
			color_index++;
		}
		if (start2.y <= 0 || end2.y >= buffer.height) {
			dy = -dy;
			color_index++;
		}
		if (color_index >= 4) {
			color_index = 0;
		}

		nanosleep((struct timespec[]){ { 0, (int)5e7 } }, NULL);
	}

	fbgl_destroy(&buffer);
	return 0;
}
