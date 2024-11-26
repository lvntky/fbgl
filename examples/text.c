#define FBGL_IMPLEMENTATION
#include "fbgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for usleep
#include <stdint.h>
#include <string.h>

// Function to save framebuffer as PPM image
int save_framebuffer_as_ppm(fbgl_t *fb, const char *filename)
{
	FILE *fp = fopen(filename, "wb");
	if (!fp) {
		perror("Error opening file");
		return -1;
	}

	// Write PPM header (P6 format - binary RGB)
	fprintf(fp, "P6\n%zu %zu\n255\n", fb->width, fb->height);

	// Allocate buffer for pixel data
	uint8_t *pixel_buffer = malloc(fb->width * fb->height * 3);
	if (!pixel_buffer) {
		perror("Memory allocation error");
		fclose(fp);
		return -1;
	}

	// Convert framebuffer to RGB
	for (size_t y = 0; y < fb->height; y++) {
		for (size_t x = 0; x < fb->width; x++) {
			uint32_t pixel = fb->pixels[y * fb->width + x];

			// Extract RGB components (assuming 32-bit ARGB or RGB)
			uint8_t r = (pixel >> 16) & 0xFF;
			uint8_t g = (pixel >> 8) & 0xFF;
			uint8_t b = pixel & 0xFF;

			// Store in buffer for PPM
			pixel_buffer[(y * fb->width + x) * 3] = r;
			pixel_buffer[(y * fb->width + x) * 3 + 1] = g;
			pixel_buffer[(y * fb->width + x) * 3 + 2] = b;
		}
	}

	// Write pixel data
	fwrite(pixel_buffer, 1, fb->width * fb->height * 3, fp);

	// Cleanup
	free(pixel_buffer);
	fclose(fp);

	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <font_file> [output_image]\n",
			argv[0]);
		return 1;
	}

	fbgl_t fb;
	fbgl_init("/dev/fb0", &fb);
	fbgl_set_bg(&fb, 0xFFFFFF);

	// Load font
	fbgl_psf1_font_t *font = fbgl_load_psf1_font(argv[1]);
	if (!font) {
		fprintf(stderr, "Failed to load font\n");
		fbgl_destroy(&fb);
		return 1;
	}

	// Text to render
	const char *text = "Hello, fbgl!";

	// Calculate text width
	size_t text_width = strlen(text) * 8;

	// Calculate centered position
	int x = (fb.width - 8) / 2;
	int y = (fb.height - 16) / 2;

	// Render centered text
	fbgl_render_psf1_text(&fb, font, text, x, y, 0xFF0000);

	// Save screenshot
	const char *output_filename = (argc > 2) ? argv[2] :
						   "fbgl_screenshot.ppm";
	if (save_framebuffer_as_ppm(&fb, output_filename) == 0) {
		//        printf("Screenshot saved as %s\n", output_filename);
	}

	// Wait for a bit to show the image
	size_t framerate = 30 * 30;
	for (size_t i = 0; i < framerate; i++) {
		usleep(50000);
	}

	// Cleanup
	fbgl_destroy_psf1_font(font);
	fbgl_destroy(&fb);
	return 0;
}
