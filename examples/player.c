#define FBGL_IMPLEMENTATION
#include "fbgl.h"

#include <stdio.h>
#include <unistd.h>

#define PLAYER_SPEED 1

typedef struct {
	int x;
	int y;
} Player;

int main(int argc, char *argv[])
{
	// Check for font file argument
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <psf1_font_file>\n", argv[0]);
		return 1;
	}

	// Load PSF1 font
	fbgl_psf1_font_t *font = fbgl_load_psf1_font(argv[1]);
	if (!font) {
		fprintf(stderr, "Failed to load PSF1 font from %s\n", argv[1]);
		return 1;
	}

	// Initialize framebuffer
	fbgl_t fb;
	if (fbgl_init(NULL, &fb) != 0) {
		fprintf(stderr, "Failed to initialize framebuffer\n");
		fbgl_destroy_psf1_font(font);
		return 1;
	}

	// Initialize keyboard
	if (fbgl_keyboard_init() != 0) {
		fprintf(stderr, "Failed to initialize keyboard\n");
		fbgl_destroy(&fb);
		fbgl_destroy_psf1_font(font);
		return 1;
	}

	// Create a player
	Player player = { .x = fb.width / 2, .y = fb.height / 2 };

	// Game loop
	while (1) {
		// Clear the screen
		fbgl_set_bg(&fb, 0x000000);

		// Get key input
		fbgl_key_t key = fbgl_get_key();

		// Handle player movement
		switch (key) {
		case FBGL_KEY_UP:
			player.y = (player.y - PLAYER_SPEED < 0) ?
					   0 :
					   player.y - PLAYER_SPEED;
			break;
		case FBGL_KEY_DOWN:
			player.y = (player.y + PLAYER_SPEED >= fb.height) ?
					   fb.height - 1 :
					   player.y + PLAYER_SPEED;
			break;
		case FBGL_KEY_LEFT:
			player.x = (player.x - PLAYER_SPEED < 0) ?
					   0 :
					   player.x - PLAYER_SPEED;
			break;
		case FBGL_KEY_RIGHT:
			player.x = (player.x + PLAYER_SPEED >= fb.width) ?
					   fb.width - 1 :
					   player.x + PLAYER_SPEED;
			break;
		case FBGL_KEY_ESCAPE:
			// Exit the program
			goto cleanup;
		default:
			break;
		}

		// Draw the player (as a small white rectangle)
		fbgl_point_t top_left = { .x = player.x - 5,
					  .y = player.y - 5 };
		fbgl_point_t bottom_right = { .x = player.x + 5,
					      .y = player.y + 5 };
		fbgl_draw_rectangle_filled(top_left, bottom_right,
					   FBGL_RGB(255, 255, 255), &fb);

		// Display debug info
		char fps_text[32];
		char pos_text[32];
		float fps = fbgl_get_fps();

		snprintf(fps_text, sizeof(fps_text), "FPS: %.2f", fps);
		snprintf(pos_text, sizeof(pos_text), "POS: %d, %d", player.x,
			 player.y);

		// Render text using the loaded PSF1 font
		fbgl_render_psf1_text(&fb, font, fps_text, 10, 10,
				      FBGL_RGB(0, 255, 0));
		fbgl_render_psf1_text(&fb, font, pos_text, 10, 30,
				      FBGL_RGB(255, 0, 0));

		// Small delay to control frame rate
		usleep(16666); // ~60 FPS
	}

cleanup:
	// Cleanup

	fbgl_destroy(&fb);
	fbgl_destroy_psf1_font(font);

	return 0;
}
