// fbgl.h - v0.1.0 - public domain Levent Kaya 2024
// FBGL - Framebuffer Graphics Library
//
// This file provides both the interface and the implementation.
// To instantiate the implementation,
//      #define FBGL_IMPLEMENTATION
// in *ONE* source file, before #including this file.
//
//
// History:
//  - 0.1.0 First public release
//
// Status:
// 24/11/2024	texture rendering implemented
//
// Contributors:
//  @lvntky
//  @dario-loi
//
// LICENSE
//
//   See end of file for license information.

#ifndef __FBGL_H__
#define __FBGL_H__

#define VERSION "0.1.0"
#define NAME "FBGL"
#define DEFAULT_FB "/dev/fb0"
#define FBGL_MAX_KEYS 256 // Maximum number of keys to track

#include <fcntl.h>
#include <linux/fb.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>

#ifdef FBGL_USE_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#endif // FBGL_USE_FREETYPE

/**
 * Structs
 */
typedef struct fbgl {
	int32_t width;
	int32_t height;
	int32_t fd;
	uint32_t screen_size;
	uint32_t *pixels;
	struct fb_var_screeninfo vinfo; // Variable screen information
	struct fb_fix_screeninfo finfo; // Fixed screen information
} fbgl_t;

typedef struct fbgl_window {
	int32_t x; // Top-left x-coordinate of the window
	int32_t y; // Top-left y-coordinate of the window
	uint32_t width; // Width of the window
	uint32_t height; // Height of the window
	fbgl_t *fb; // Pointer to the framebuffer context
} fbgl_window_t;

typedef struct fbgl_point {
	int32_t x;
	int32_t y;
} fbgl_point_t;

typedef struct fbgl_tga_texture {
	uint16_t width;
	uint16_t height;
	uint32_t *data;
} fbgl_tga_texture_t;

typedef struct fbgl_psf1_font {
	uint8_t magic[2]; // Magic number (0x36, 0x04 for PSF1)
	uint8_t mode; // Mode (0 = 256 glyphs, 1 = 512 glyphs)
	uint8_t char_height; // Character height in pixels
	uint8_t *glyphs; // Pointer to glyph data
	uint16_t glyph_count; // Number of glyphs (calculated from mode)
	uint16_t char_width; // Character width in pixels (always 8 for PSF1)
} fbgl_psf1_font_t;

typedef struct fbgl_keyboard {
	bool keys[FBGL_MAX_KEYS]; // Current state of each key
	bool prev_keys[FBGL_MAX_KEYS]; // Previous state of each key
	bool is_initialized; // Track if keyboard is initialized
} fbgl_keyboard_t;

/**
 * Key state function and variables
 *
 */
static struct termios orig_termios;
static fbgl_keyboard_t keyboard = { 0 };
static struct timespec previous_frame_time = { 0 };

#ifdef FBGL_HIDE_CURSOR
#include <linux/kd.h>
int fbgl_hide_cursor(int fd)
{
	int tty_fd = open("/dev/tty0", O_RDWR);
	if (tty_fd == -1) {
		perror("Error opening /dev/tty0");
		return -1;
	}

	if (ioctl(tty_fd, KDSETMODE, KD_GRAPHICS) == -1) {
		perror("Error setting graphics mode");
		close(tty_fd);
		return -1;
	}

	close(tty_fd);
	return 0;
}
#endif // FBGL_HIDE_CURSOR

#ifdef __cplusplus
extern "C" {
#endif

/**
 * General purpose methods
 */
char const *fbgl_name_info(void);
char const *fbgl_version_info(void);
void fbgl_enable_raw_mode(void);
void fbgl_disable_raw_mode(void);
void fbgl_cleanup(int sig);
int fbgl_check_esc_key(void);
void fbgl_set_signal_handlers(void);
float fbgl_get_fps(void);

/*Create and destroy methods*/
int fbgl_init(const char *device, fbgl_t *fb);
void fbgl_destroy(fbgl_t *fb);

/**
 * Drawing functions
 */
void fbgl_clear(uint32_t color);
void fbgl_put_pixel(int x, int y, uint32_t color, fbgl_t *fb);
void fbgl_draw_line(fbgl_point_t x, fbgl_point_t y, uint32_t color, fbgl_t *fb);

/**
 * Access framebuffer data methods
 */
uint32_t *fb_get_data(fbgl_t const *fb);
uint32_t fb_get_width(fbgl_t const *fb);
uint32_t fb_get_height(fbgl_t const *fb);

/**
 * Shapes
 */
void fbgl_draw_rectangle_outline(fbgl_point_t top_left,
				 fbgl_point_t bottom_right, uint32_t color,
				 fbgl_t *fb);
void fbgl_draw_rectangle_filled(fbgl_point_t top_left,
				fbgl_point_t bottom_right, uint32_t color,
				fbgl_t *fb);

/**
 * texture
 */
fbgl_tga_texture_t *fbgl_load_tga_texture(const char *path);
void fbgl_destroy_texture(fbgl_tga_texture_t *texture);
void fbgl_draw_texture(fbgl_t *fb, fbgl_tga_texture_t const *texture, int32_t x,
		       int32_t y);

/**
* Text
*/
fbgl_psf1_font_t *fbgl_load_psf1_font(const char *path);
void fbgl_destroy_psf1_font(fbgl_psf1_font_t *font);
void fbgl_render_psf1_text(fbgl_t *fb, fbgl_psf1_font_t *font, const char *text,
			   int x, int y, uint32_t color);
/**
 * Keyboard
 */
int fbgl_keyboard_init(void);
void fbgl_keyboard_clean(void);
void fbgl_keyboard_update(void);
bool fbgl_key_pressed(unsigned char key);
bool fbgl_key_released(unsigned char key);
bool fbgl_key_down(unsigned char key);

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
	if (!fb) {
		fprintf(stderr, "Error: fbgl_t pointer is NULL.");
		return -1;
	}

	fb->fd = device == NULL ? open(DEFAULT_FB, O_RDWR) :
				  open(device, O_RDWR);
	if (fb->fd == -1) {
		perror("Error openning framebuffer device");
		return -1;
	}

	if (ioctl(fb->fd, FBIOGET_FSCREENINFO, &fb->finfo) == -1) {
		perror("Error: Reading fixed information.");
		close(fb->fd);
		return -1;
	}
	if (ioctl(fb->fd, FBIOGET_VSCREENINFO, &fb->vinfo) == -1) {
		perror("Error reading variable information");
		close(fb->fd);
		return -1;
	}

#ifdef FBGL_HIDE_CURSOR
	fbgl_hide_cursor(fb->fd);
	fbgl_set_signal_handlers();
	fbgl_enable_raw_mode();
#endif // FBGL_HIDE_CURSOR

	fb->width = fb->vinfo.xres;
	fb->height = fb->vinfo.yres;
	fb->screen_size = fb->finfo.smem_len;

	// Map framebuffer to memory
	fb->pixels = (uint32_t *)mmap(NULL, fb->screen_size,
				      PROT_READ | PROT_WRITE, MAP_SHARED,
				      fb->fd, 0);
	if (fb->pixels == MAP_FAILED) {
		perror("Error mapping framebuffer device to memory");
		close(fb->fd);
		return -1;
	}

	return 0;
}

void fbgl_destroy(fbgl_t *fb)
{
	if (!fb || fb->fd == -1) {
		fprintf(stderr,
			"Error: framebuffer not initialized or already destroyed.\n");
		return;
	}

	if (fb->pixels && fb->pixels != MAP_FAILED) {
		munmap(fb->pixels, fb->screen_size);
	}

	close(fb->fd);
	fb->fd = -1;

#ifdef FBGL_HIDE_CURSOR
	fbgl_disable_raw_mode();
#endif // FBGL_HIDE_CURSOR
}

void fbgl_set_bg(fbgl_t *fb, uint32_t color)
{
#ifdef DEBUG
	if (!fb || fb->fd == -1) {
		fprintf(stderr, "Error: framebuffer not initialized.\n");
		return;
	}
#endif // DEBUG

	// Fill the entire framebuffer with the specified color
	for (int32_t i = 0; i < fb->width * fb->height; i++) {
		fb->pixels[i] = color;
	}
}

void fbgl_put_pixel(int x, int y, uint32_t color, fbgl_t *fb)
{
#ifdef FBGL_VALIDATE_PUT_PIXEL
	if (!fb || !fb->pixels) {
		fprintf(stderr, "Error: framebuffer not initialized.\n");
		return;
	}

	if (x < 0 || x >= fb->width || y < 0 || y >= fb->height) {
		return; // Ignore out-of-bound coordinates
	}
#endif // FBGL_VALIDATE_PUT_PIXEL

	const size_t index = y * fb->width + x;
	fb->pixels[index] = color;
}

void fbgl_enable_raw_mode(void)
{
	struct termios raw;

	if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
		perror("tcgetattr");
		exit(EXIT_FAILURE);
	}
	raw = orig_termios;
	raw.c_lflag &= ~(ECHO | ICANON);
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
		perror("tcsetattr");
		exit(EXIT_FAILURE);
	}
}

void fbgl_disable_raw_mode(void)
{
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
		perror("tcsetattr");
	}
}

void fbgl_cleanup(int sig)
{
	fbgl_disable_raw_mode();
	printf("\033[2J\033[H"); // Clear the terminal screen and move the cursor to top-left
	exit(sig);
}

int fbgl_check_esc_key(void)
{
	char c;
	struct timeval tv = { 0, 0 }; // Timeout of 0, to poll immediately
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);

	// Check if there's input available
	if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) == -1) {
		perror("select");
		return 0;
	}

	if (FD_ISSET(STDIN_FILENO, &fds)) {
		if (read(STDIN_FILENO, &c, 1) == -1) {
			perror("read");
			return 0;
		}
		return c == 27; // ASCII value of the `Esc` key
	}

	return 0;
}

void fbgl_set_signal_handlers(void)
{
	struct sigaction sa;
	sa.sa_handler = fbgl_cleanup;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGINT, &sa, NULL) == -1 ||
	    sigaction(SIGTERM, &sa, NULL) == -1) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}
}

void fbgl_draw_line(fbgl_point_t x, fbgl_point_t y, uint32_t color,
		    fbgl_t *buffer)
{
	const int32_t dx = abs(y.x - x.x);
	const int32_t dy = abs(y.y - x.y);

	const int32_t sx = (x.x < y.x) ? 1 : -1;
	const int32_t sy = (x.y < y.y) ? 1 : -1;

	int32_t err = dx - dy;

	while (1) {
		// Set the pixel at the current position
		fbgl_put_pixel(x.x, x.y, color, buffer);

		// If we've reached the end point, break
		if (x.x >= y.x && x.y >= y.y)
			break;

		const int32_t e2 = 2 * err;

		if (e2 > -dy) {
			err -= dy;
			x.x += sx;
		}

		if (e2 < dx) {
			err += dx;
			x.y += sy;
		}
	}
}
void fbgl_draw_rectangle_outline(fbgl_point_t top_left,
				 fbgl_point_t bottom_right, uint32_t color,
				 fbgl_t *fb)
{
	// Top horizontal line
	for (int x = top_left.x; x < bottom_right.x; x++) {
		fbgl_put_pixel(x, top_left.y, color, fb);
	}

	// Bottom horizontal line
	for (int x = top_left.x; x < bottom_right.x; x++) {
		fbgl_put_pixel(x, bottom_right.y - 1, color, fb);
	}

	// Left vertical line
	for (int y = top_left.y; y < bottom_right.y; y++) {
		fbgl_put_pixel(top_left.x, y, color, fb);
	}

	// Right vertical line
	for (int y = top_left.y; y < bottom_right.y; y++) {
		fbgl_put_pixel(bottom_right.x - 1, y, color, fb);
	}
}

void fbgl_draw_rectangle_filled(fbgl_point_t top_left,
				fbgl_point_t bottom_right, uint32_t color,
				fbgl_t *fb)
{
	for (int32_t y = top_left.y; y < bottom_right.y; y++) {
		// Manually set each pixel in the row
		for (int32_t x = top_left.x; x < bottom_right.x; x++) {
			fbgl_put_pixel(x, y, color, fb);
		}
	}
}
fbgl_tga_texture_t *fbgl_load_tga_texture(const char *path)
{
	FILE *file = fopen(path, "rb");
	if (!file) {
		perror("Unable to open texture file");
		return NULL;
	}

	// TGA header structure
	uint8_t header[18];
	if (fread(header, 1, sizeof(header), file) != sizeof(header)) {
		perror("Error reading TGA header");
		fclose(file);
		return NULL;
	}

	// Allocate texture structure
	fbgl_tga_texture_t *texture =
		(fbgl_tga_texture_t *)malloc(sizeof(fbgl_tga_texture_t));
	if (!texture) {
		perror("Failed to allocate texture structure");
		fclose(file);
		return NULL;
	}

	// Extract dimensions from header
	texture->width = header[12] | (header[13] << 8);
	texture->height = header[14] | (header[15] << 8);
	uint8_t bits_per_pixel = header[16];
	uint8_t image_descriptor = header[17];

	// Verify format support
	if (bits_per_pixel != 24 && bits_per_pixel != 32) {
		fprintf(stderr,
			"Unsupported TGA bit depth: %d (only 24 and 32-bit supported)\n",
			bits_per_pixel);
		free(texture);
		fclose(file);
		return NULL;
	}

	// Skip image ID field
	if (header[0]) {
		fseek(file, header[0], SEEK_CUR);
	}

	// Allocate pixel data
	size_t pixel_count = texture->width * texture->height;
	texture->data = (uint32_t *)malloc(pixel_count * sizeof(uint32_t));
	if (!texture->data) {
		perror("Failed to allocate pixel data");
		free(texture);
		fclose(file);
		return NULL;
	}

	// Read pixel data
	uint8_t *pixel_buffer = (uint8_t *)malloc(bits_per_pixel / 8);
	if (!pixel_buffer) {
		perror("Failed to allocate pixel buffer");
		free(texture->data);
		free(texture);
		fclose(file);
		return NULL;
	}

	// Determine if image is flipped (based on image descriptor)
	bool bottom_up = !(image_descriptor & 0x20);

	for (size_t i = 0; i < pixel_count; i++) {
		size_t pixel_index =
			bottom_up ?
				(texture->height - 1 - (i / texture->width)) *
						texture->width +
					(i % texture->width) :
				i;

		if (fread(pixel_buffer, 1, bits_per_pixel / 8, file) !=
		    bits_per_pixel / 8) {
			perror("Error reading pixel data");
			free(pixel_buffer);
			free(texture->data);
			free(texture);
			fclose(file);
			return NULL;
		}

		// Convert BGR(A) to RGBA
		uint32_t pixel = 0xFF000000; // Default alpha to 255
		pixel |= pixel_buffer[2] << 16; // R
		pixel |= pixel_buffer[1] << 8; // G
		pixel |= pixel_buffer[0]; // B
		if (bits_per_pixel == 32) {
			pixel = (pixel & 0x00FFFFFF) |
				(pixel_buffer[3] << 24); // A
		}

		texture->data[pixel_index] = pixel;
	}

	free(pixel_buffer);
	fclose(file);
	return texture;
}

void fbgl_destroy_texture(fbgl_tga_texture_t *texture)
{
	if (texture) {
		free(texture->data);
		free(texture);
	}
}

void fbgl_draw_texture(fbgl_t *fb, fbgl_tga_texture_t const *texture, int32_t x,
		       int32_t y)
{
	if (!fb || !texture || !texture->data) {
		return;
	}

	for (int ty = 0; ty < texture->height; ty++) {
		for (int tx = 0; tx < texture->width; tx++) {
			int screen_x = x + tx;
			int screen_y = y + ty;

			// Skip if outside screen bounds
			if (screen_x < 0 || screen_x >= fb->width ||
			    screen_y < 0 || screen_y >= fb->height) {
				continue;
			}

			uint32_t pixel =
				texture->data[ty * texture->width + tx];
			// Only draw if pixel is not fully transparent
			if ((pixel & 0xFF000000) != 0) {
				fbgl_put_pixel(screen_x, screen_y, pixel, fb);
			}
		}
	}
}

uint32_t fb_get_width(fbgl_t const *fb)
{
	return fb->width;
}

uint32_t fb_get_height(fbgl_t const *fb)
{
	return fb->height;
}

uint32_t *fb_get_data(fbgl_t const *fb)
{
	return fb->pixels;
}

float fbgl_get_fps(void)
{
	struct timespec current_time;
	clock_gettime(CLOCK_MONOTONIC, &current_time);

	if (previous_frame_time.tv_sec == 0 &&
	    previous_frame_time.tv_nsec == 0) {
		previous_frame_time = current_time;
		return 0.0f;
	}

	double time_diff =
		(current_time.tv_sec - previous_frame_time.tv_sec) +
		(current_time.tv_nsec - previous_frame_time.tv_nsec) / 1e9;

	previous_frame_time = current_time;

	if (time_diff > 0.0) {
		return 1.0 / time_diff;
	} else {
		return 0.0f; // Avoid division by zero
	}
}

fbgl_psf1_font_t *fbgl_load_psf1_font(const char *path)
{
	FILE *file = fopen(path, "rb");
	if (!file) {
		perror("Failed to open font file");
		return NULL;
	}

	// Allocate memory for the font structure
	fbgl_psf1_font_t *font = malloc(sizeof(fbgl_psf1_font_t));
	if (!font) {
		perror("Failed to allocate memory for font");
		fclose(file);
		return NULL;
	}

	// Read the header (4 bytes)
	uint8_t header[4];
	if (fread(header, 1, sizeof(header), file) != sizeof(header)) {
		perror("Failed to read font header");
		free(font);
		fclose(file);
		return NULL;
	}

	// Verify magic number
	if (header[0] != 0x36 || header[1] != 0x04) {
		fprintf(stderr, "Invalid PSF1 magic number\n");
		free(font);
		fclose(file);
		return NULL;
	}

	// Populate the font structure
	font->magic[0] = header[0];
	font->magic[1] = header[1];
	font->mode = header[2];
	font->char_height = header[3];
	font->glyph_count = (font->mode & 0x01) ? 512 :
						  256; // Determine glyph count
	font->char_width = 8; // PSF1 glyphs are always 8 pixels wide

	// Allocate memory for glyphs
	size_t glyph_data_size = font->glyph_count * font->char_height;
	font->glyphs = malloc(glyph_data_size);
	if (!font->glyphs) {
		perror("Failed to allocate memory for glyphs");
		free(font);
		fclose(file);
		return NULL;
	}

	// Read glyph data
	if (fread(font->glyphs, 1, glyph_data_size, file) != glyph_data_size) {
		perror("Failed to read glyph data");
		free(font->glyphs);
		free(font);
		fclose(file);
		return NULL;
	}

	fclose(file);
	return font;
}

void fbgl_destroy_psf1_font(fbgl_psf1_font_t *font)
{
	if (font) {
		free(font->glyphs);
		free(font);
	}
}

void fbgl_render_psf1_text(fbgl_t *fb, fbgl_psf1_font_t *font, const char *text,
			   int x, int y, uint32_t color)
{
	if (!fb || !font || !text)
		return;

	int cursor_x = x;
	int cursor_y = y;

	for (const char *c = text; *c; c++) {
		uint8_t glyph_index = (uint8_t)*c;

		// Ensure glyph index is within range
		if (glyph_index >= font->glyph_count)
			glyph_index = 0; // Default to space or undefined glyph

		// Locate the glyph in the glyph table
		uint8_t *glyph = font->glyphs + glyph_index * font->char_height;

		// Render the glyph
		for (int row = 0; row < font->char_height; row++) {
			for (int col = 0; col < font->char_width; col++) {
				// Check if the bit is set in the glyph
				if (glyph[row] & (0x80 >> col)) {
					fbgl_put_pixel(cursor_x + col,
						       cursor_y + row, color,
						       fb);
				}
			}
		}

		// Move to the next character position
		cursor_x += font->char_width;
	}
}

#endif // FBGL_IMPLEMENTATION

#ifdef __cplusplus
} // extern "C"
#endif
#endif // __FBGL_H__

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2024 Levent Kaya
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/
