#include <stdio.h>
#include <stdbool.h>
#define FBGL_IMPLEMENTATION
#include "fbgl.h"

#define RECT_WIDTH 50
#define RECT_HEIGHT 30
#define MOVE_STEP 10
#define BG_COLOR 0x000000 // Black
#define RECT_COLOR 0xFFFFFF // White

int main(void)
{
    // Initialize framebuffer
    fbgl_t framebuffer;
    if (fbgl_init(NULL, &framebuffer) != 0) {
        fprintf(stderr, "Failed to initialize framebuffer\n");
        return EXIT_FAILURE;
    }

    // Initialize keyboard
    if (fbgl_keyboard_init() != 0) {
        fprintf(stderr, "Failed to initialize keyboard\n");
        fbgl_destroy(&framebuffer);
        return EXIT_FAILURE;
    }

    atexit(fbgl_keyboard_clean); // Cleanup on exit
    atexit((void (*)(void))fbgl_destroy, &framebuffer);

    // Initial rectangle position
    fbgl_point_t top_left = {framebuffer.width / 2 - RECT_WIDTH / 2, framebuffer.height / 2 - RECT_HEIGHT / 2};
    fbgl_point_t bottom_right = {top_left.x + RECT_WIDTH, top_left.y + RECT_HEIGHT};

    // Main loop
    while (!fbgl_check_esc_key()) {
        // Update keyboard state
        fbgl_keyboard_update();

        // Clear screen
        fbgl_set_bg(&framebuffer, BG_COLOR);

        // Move rectangle based on key input
        if (fbgl_key_down('w')) {
            top_left.y -= MOVE_STEP;
            bottom_right.y -= MOVE_STEP;
        }
        if (fbgl_key_down('s')) {
            top_left.y += MOVE_STEP;
            bottom_right.y += MOVE_STEP;
        }
        if (fbgl_key_down('a')) {
            top_left.x -= MOVE_STEP;
            bottom_right.x -= MOVE_STEP;
        }
        if (fbgl_key_down('d')) {
            top_left.x += MOVE_STEP;
            bottom_right.x += MOVE_STEP;
        }

        // Ensure rectangle stays within screen bounds
        if (top_left.x < 0) {
            top_left.x = 0;
            bottom_right.x = RECT_WIDTH;
        }
        if (top_left.y < 0) {
            top_left.y = 0;
            bottom_right.y = RECT_HEIGHT;
        }
        if (bottom_right.x > framebuffer.width) {
            bottom_right.x = framebuffer.width;
            top_left.x = framebuffer.width - RECT_WIDTH;
        }
        if (bottom_right.y > framebuffer.height) {
            bottom_right.y = framebuffer.height;
            top_left.y = framebuffer.height - RECT_HEIGHT;
        }

        // Draw the rectangle
        fbgl_draw_rectangle_filled(top_left, bottom_right, RECT_COLOR, &framebuffer);

        // Refresh frame (add a delay if needed)
        usleep(16000); // ~60 FPS
    }

    // Cleanup
    fbgl_keyboard_clean();
    fbgl_destroy(&framebuffer);

    return EXIT_SUCCESS;
}
