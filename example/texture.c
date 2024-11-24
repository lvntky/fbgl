#define FBGL_IMPLEMENTATION
#include "../fbgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for usleep

int main(int argc, char **argv)
{
	// Initialize framebuffer
	fbgl_t framebuffer;
	if (fbgl_init(NULL, &framebuffer) != 0) {
		fprintf(stderr, "Failed to initialize framebuffer.\n");
		return EXIT_FAILURE;
	}

	// Load a TGA texture
	const char *texture_path = argv[1];
	fbgl_tga_texture_t *texture = fbgl_load_tga_texture(texture_path);
	if (!texture) {
		fprintf(stderr, "Failed to load texture.\n");
		fbgl_destroy(&framebuffer);
		return EXIT_FAILURE;
	}

	// Set a background color (e.g., black)
	fbgl_set_bg(&framebuffer, 0x000000); // Clear the framebuffer to black

	// Texture movement parameters
	int texture_x = 0; // Initial horizontal position of the texture
	int texture_y = 100; // Initial vertical position of the texture
	int dx = 5; // Horizontal speed (adjust for desired marquee speed)
	int dy = 3; // Vertical speed (adjust for desired marquee speed)

	// Main rendering loop
	int framesize = 30 * 30;
	while (framesize) {
		// Clear the framebuffer (set background)
		fbgl_set_bg(&framebuffer, 0x000000);

		// Draw the texture at the current position
		fbgl_draw_texture(&framebuffer, texture, texture_x, texture_y);

		// Move the texture by updating its position
		texture_x += dx;
		texture_y += dy;

		// Reverse direction if the texture hits the screen boundary (X-axis)
		if (texture_x <= 0 ||
		    texture_x + texture->width >= framebuffer.width) {
			dx = -dx; // Reverse horizontal direction when hitting the left or right edge
		}

		// Reverse direction if the texture hits the screen boundary (Y-axis)
		if (texture_y <= 0 ||
		    texture_y + texture->height >= framebuffer.height) {
			dy = -dy; // Reverse vertical direction when hitting the top or bottom edge
		}

		usleep(50000); // Delay to make the marquee effect visible (adjust as needed)
		framesize--;
	}

	// Wait for the user to press the escape key before exiting
	printf("Press ESC to exit...\n");
	while (!fbgl_check_esc_key()) {
		// You can add additional rendering logic here if needed
	}

	// Clean up
	fbgl_destroy_texture(texture);
	fbgl_destroy(&framebuffer);

	return EXIT_SUCCESS;
}
