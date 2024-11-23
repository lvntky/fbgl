#ifndef __FBGL_H__
#define __FBGL_H__

#define VERSION "0.1.0"
#define NAME "FBGL"
#define DEFAULT_FB "/dev/fb0"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/fb.h>
#include <stddef.h>
#include <stdio.h>
#include <termios.h>
#include <signal.h>

#ifdef FBGL_USE_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#endif //FBGL_USE_FREETYPE

static struct termios orig_termios;

/**
* Structs
*/
typedef struct fbgl {
	int width;
	int height;
	int fd;
	size_t screen_size;
	uint32_t *pixels;
	struct fb_var_screeninfo vinfo; // Variable screen information
	struct fb_fix_screeninfo finfo; // Fixed screen information
} fbgl_t;

typedef struct fbgl_window {
	int x; // Top-left x-coordinate of the window
	int y; // Top-left y-coordinate of the window
	int width; // Width of the window
	int height; // Height of the window
	fbgl_t *fb; // Pointer to the framebuffer context
} fbgl_window_t;

typedef struct fbgl_psf2_header {
	uint8_t magic[2]; // Magic number, should be {0x72, 0xB5}
	uint8_t version; // Version of PSF2 (usually 0)
	uint8_t header_size; // Size of the header (usually 32 bytes)
	uint16_t flags; // Flags (usually 0)
	uint16_t numglyphs; // Number of glyphs (characters)
	uint16_t bytes_per_glyph; // Number of bytes per glyph (depends on font size)
	uint16_t height; // Height of each character in pixels
	uint16_t width; // Width of each character in pixels
	uint8_t *glyphs; // Pointer to the glyph data
} fbgl_psf2_header_t;

typedef struct fbgl_point {
	size_t x;
	size_t y;
} fbgl_point_t;

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
#endif //FBGL_HIDE_CURSOR

#ifdef FBGL_USE_FREETYPE
FT_Library fbgl_freetype_init();
void fbgl_freetype_cleanup(FT_Library library);
FT_Face fbgl_load_font(FT_Library library, const char *font_path,
		       int font_size);
void fbgl_render_freetype_text(fbgl_t *fb, FT_Library library, FT_Face face,
			       const char *text, int x, int y);
#endif //FBGL_USE_FREETYPE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * General purpose methods
 */
char const *fbgl_name_info(void);
char const *fbgl_version_info(void);
void fbgl_enable_raw_mode();
void fbgl_disable_raw_mode();
void fbgl_cleanup(int sig);
int fbgl_check_esc_key();
void fbgl_set_signal_handlers();

/*Create and destroy methods*/
int fbgl_init(const char *device, fbgl_t *fb);
void fbgl_destroy(fbgl_t *fb);

/**
* Drawing functions
*/
void fbgl_clear(uint32_t color);
void fbgl_put_pixel(int x, int y, uint32_t color, fbgl_t *fb);
void fbgl_draw_line(fbgl_point_t x, fbgl_point_t y, uint32_t color, fbgl_t *fb);
void fbgl_set_bg();

/**
* Display methods
*/
void fbgl_display();

/**
* Access framebuffer data methods
*/
uint32_t *fb_get_data(void);
int fb_get_width(void);
int fb_get_height(void);

/**
* Shapes
*/
void fbgl_draw_rectangle_outline(fbgl_point_t top_left,
				 fbgl_point_t bottom_right, uint32_t color,
				 fbgl_t *fb);
void fbgl_draw_rectangle_filled(fbgl_point_t top_left,
				fbgl_point_t bottom_right, uint32_t color,
				fbgl_t *fb);

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
#endif //FBGL_HIDE_CURSOR

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
	if (!fb || fb->fd == -1) {
		fprintf(stderr, "Error: framebuffer not initialized.\n");
		return;
	}

	// Fill the entire framebuffer with the specified color
	for (size_t i = 0; i < (fb->width * fb->height); ++i) {
		fb->pixels[i] = color;
	}
}

void fbgl_put_pixel(int x, int y, uint32_t color, fbgl_t *fb)
{
	if (!fb || !fb->pixels) {
		fprintf(stderr, "Error: framebuffer not initialized.\n");
		return;
	}

	if (x < 0 || x >= fb->width || y < 0 || y >= fb->height) {
		return; // Ignore out-of-bound coordinates
	}

	size_t index = y * fb->width + x;
	fb->pixels[index] = color;
}

void fbgl_enable_raw_mode()
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

void fbgl_disable_raw_mode()
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

int fbgl_check_esc_key()
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

void fbgl_set_signal_handlers()
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

#endif

#ifdef FBGL_USE_FREETYPE
FT_Library fbgl_freetype_init()
{
	FT_Library library;
	if (FT_Init_FreeType(&library)) {
		fprintf(stderr, "Could not init FreeType Library\n");
		return NULL;
	}
	return library;
}

void fbgl_freetype_cleanup(FT_Library library)
{
	if (library) {
		FT_Done_FreeType(library);
	}
}

FT_Face fbgl_load_font(FT_Library library, const char *font_path, int font_size)
{
	FT_Face face;
	if (FT_New_Face(library, font_path, 0, &face)) {
		fprintf(stderr, "Could not open font: %s\n", font_path);
		return NULL;
	}
	FT_Set_Pixel_Sizes(face, 0, font_size); // Set font size
	return face;
}

void fbgl_render_freetype_text(fbgl_t *fb, FT_Library library, FT_Face face,
			       const char *text, int x, int y)
{
	while (*text) {
		FT_Load_Char(face, *text, FT_LOAD_RENDER);
		FT_Bitmap bitmap = face->glyph->bitmap;

		// Draw the bitmap to framebuffer
		for (int j = 0; j < bitmap.rows; j++) {
			for (int i = 0; i < bitmap.width; i++) {
				if (bitmap.buffer[j * bitmap.width +
						  i]) { // Check pixel is not empty
					fbgl_put_pixel(x + i, y + j, 0xFF0000,
						       fb); // Draw white pixel
				}
			}
		}
		x += face->glyph->advance.x >>
		     6; // Move to the next character position
		++text;
	}
}

#endif // FBGL_USE_FREETYPE
void fbgl_draw_line(fbgl_point_t x, fbgl_point_t y, uint32_t color,
		    fbgl_t *buffer)
{
	int dx = abs(y.x - x.x);
	int dy = abs(y.y - x.y);

	int sx = (x.x < y.x) ? 1 : -1;
	int sy = (x.y < y.y) ? 1 : -1;

	int err = dx - dy;

	while (1) {
		// Set the pixel at the current position
		fbgl_put_pixel(x.x, x.y, color, buffer);

		// If we've reached the end point, break
		if (x.x == y.x && x.y == y.y)
			break;

		int e2 = 2 * err;

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
void fbgl_draw_rectangle_outline(fbgl_point_t top_left, fbgl_point_t bottom_right, uint32_t color, fbgl_t *fb) {
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

void fbgl_draw_rectangle_filled(fbgl_point_t top_left, fbgl_point_t bottom_right, uint32_t color, fbgl_t *fb) {
    for (int y = top_left.y; y < bottom_right.y; y++) {
        // Manually set each pixel in the row
        for (int x = top_left.x; x < bottom_right.x; x++) {
            fbgl_put_pixel(x, y, color, fb);
        }
    }
}

#ifdef __cplusplus
} // extern "C"
#endif
#endif // __FBGL_H__
