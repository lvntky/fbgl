#define FBGL_IMPLEMENTATION
#include "fbgl.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#define MAP_WIDTH 8
#define MAP_HEIGHT 8
#define TILE_SIZE 64
#define PLAYER_SPEED 2
#define TURN_SPEED 0.1f

uint32_t get_pixel(const fbgl_t *framebuffer, int x, int y) {
    return framebuffer->pixels[y * framebuffer->width + x];
}


void save_frame_as_ppm(const fbgl_t *framebuffer, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Failed to save frame as PPM.\n");
        return;
    }

    // Write PPM header
    fprintf(file, "P6\n%d %d\n255\n", framebuffer->width, framebuffer->height);

    // Write pixel data
    for (int y = 0; y < framebuffer->height; ++y) {
        for (int x = 0; x < framebuffer->width; ++x) {
            uint32_t color = get_pixel(framebuffer, x, y);
            uint8_t r = (color >> 16) & 0xFF;
            uint8_t g = (color >> 8) & 0xFF;
            uint8_t b = color & 0xFF;
            fwrite(&r, 1, 1, file);
            fwrite(&g, 1, 1, file);
            fwrite(&b, 1, 1, file);
        }
    }

    fclose(file);
}


// World map
const char WORLD_MAP[MAP_WIDTH][MAP_HEIGHT] = {
    {'#', '#', '#', '#', '#', '#', '#', '#'},
    {'#', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
    {'#', ' ', '#', '#', ' ', '#', ' ', '#'},
    {'#', ' ', '#', ' ', ' ', '#', ' ', '#'},
    {'#', ' ', '#', ' ', '#', '#', ' ', '#'},
    {'#', ' ', ' ', ' ', ' ', ' ', ' ', '#'},
    {'#', ' ', '#', '#', '#', '#', ' ', '#'},
    {'#', '#', '#', '#', '#', '#', '#', '#'}
};

// Player structure
typedef struct {
    float x, y;         // Player position
    float angle;        // Viewing angle
    float fov;          // Field of view
} Player;

// Player instance
Player player = {128.0f, 128.0f, 0.0f, M_PI / 4.0f};

// Framebuffer instance
fbgl_t framebuffer;

// Calculate shaded color based on distance
uint32_t calculate_shaded_color(uint32_t base_color, float distance) {
    float shade_factor = 1.0f / (distance + 1.0f);
    return FBGL_RGB(
        (int)((base_color >> 16 & 0xFF) * shade_factor),
        (int)((base_color >> 8 & 0xFF) * shade_factor),
        (int)((base_color & 0xFF) * shade_factor)
    );
}

// Draw a solid vertical slice for a wall
void draw_wall_slice(int x, int wall_height, uint32_t color) {
    int top = (framebuffer.height / 2) - (wall_height / 2);
    int bottom = (framebuffer.height / 2) + (wall_height / 2);

    // Clip the top and bottom to the framebuffer boundaries
    if (top < 0) top = 0;
    if (bottom >= framebuffer.height) bottom = framebuffer.height - 1;

    fbgl_point_t top_left = {x, top};
    fbgl_point_t bottom_right = {x + 1, bottom};
    fbgl_draw_rectangle_filled(top_left, bottom_right, color, &framebuffer);
}

int frame_counter = 0;
// Perform ray-casting to render the 3D view
void cast_rays() {
    for (int ray = 0; ray < framebuffer.width; ray++) {
        float ray_angle = player.angle - player.fov / 2 + player.fov * ray / framebuffer.width;

        float distance_to_wall = 0.0f;
        bool hit_wall = false;
        bool vertical_hit = false;

        float eye_x = cos(ray_angle);
        float eye_y = sin(ray_angle);

        while (!hit_wall && distance_to_wall < 16.0f) {
            distance_to_wall += 0.1f;
            int test_x = (int)(player.x / TILE_SIZE + eye_x * distance_to_wall);
            int test_y = (int)(player.y / TILE_SIZE + eye_y * distance_to_wall);

            // Check if the ray is out of bounds
            if (test_x < 0 || test_x >= MAP_WIDTH || test_y < 0 || test_y >= MAP_HEIGHT) {
                hit_wall = true;
                distance_to_wall = 16.0f;
                break;
            }

            // Check if the ray hit a wall
            if (WORLD_MAP[test_x][test_y] == '#') {
                hit_wall = true;

                // Determine if the hit was on a vertical wall
                if (fabs(eye_x) > fabs(eye_y)) {
                    vertical_hit = true;
                }
            }
        }

        // Calculate wall height
        int wall_height = (int)(framebuffer.height / distance_to_wall);

        // Choose base wall color
        uint32_t base_color = vertical_hit
                                  ? FBGL_RGB(150, 150, 255) // Bluish for vertical walls
                                  : FBGL_RGB(255, 150, 150); // Reddish for horizontal walls

        // Calculate shaded color
        uint32_t shaded_color = calculate_shaded_color(base_color, distance_to_wall);

        // Render the wall slice
        draw_wall_slice(ray, wall_height, shaded_color);
    }
}

// Main program
int main() {
    // Initialize framebuffer
    if (fbgl_init(NULL, &framebuffer) < 0) {
        fprintf(stderr, "Failed to initialize framebuffer.\n");
        return 1;
    }

    // Initialize keyboard
    if (fbgl_keyboard_init() != 0) {
        fprintf(stderr, "Failed to initialize keyboard.\n");
        fbgl_destroy(&framebuffer);
        return 1;
    }

    // Main game loop
    while (true) {
        // Set the background color to black
        fbgl_set_bg(&framebuffer, FBGL_RGB(0, 0, 0));

        // Perform ray-casting and render the scene
        cast_rays();
	char filename[64];
        snprintf(filename, sizeof(filename), "frame_%04d.ppm", frame_counter++);
        save_frame_as_ppm(&framebuffer, filename);
        // Get key input
        fbgl_key_t key = fbgl_get_key();

        // Handle player movement
        if (key == FBGL_KEY_UP) {
            float next_x = player.x + cos(player.angle) * PLAYER_SPEED;
            float next_y = player.y + sin(player.angle) * PLAYER_SPEED;

            // Prevent moving into walls
            if (WORLD_MAP[(int)(next_x / TILE_SIZE)][(int)(player.y / TILE_SIZE)] != '#') {
                player.x = next_x;
            }
            if (WORLD_MAP[(int)(player.x / TILE_SIZE)][(int)(next_y / TILE_SIZE)] != '#') {
                player.y = next_y;
            }
        } else if (key == FBGL_KEY_DOWN) {
            float next_x = player.x - cos(player.angle) * PLAYER_SPEED;
            float next_y = player.y - sin(player.angle) * PLAYER_SPEED;

            // Prevent moving into walls
            if (WORLD_MAP[(int)(next_x / TILE_SIZE)][(int)(player.y / TILE_SIZE)] != '#') {
                player.x = next_x;
            }
            if (WORLD_MAP[(int)(player.x / TILE_SIZE)][(int)(next_y / TILE_SIZE)] != '#') {
                player.y = next_y;
            }
        } else if (key == FBGL_KEY_LEFT) {
            player.angle -= TURN_SPEED;
        } else if (key == FBGL_KEY_RIGHT) {
            player.angle += TURN_SPEED;
        } else if (key == FBGL_KEY_ESCAPE) {
            // Exit the game loop
            break;
        }

        // Frame delay for ~60 FPS
        usleep(16666);
    }

    // Cleanup resources
    fbgl_destroy(&framebuffer);
    fbgl_destroy_keyboard();
    return 0;
}
