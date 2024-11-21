#define FBGL_IMPLEMENTATION
//#define FBGL_HIDE_CURSOR
#include "../fbgl.h"

#include <stdio.h>
#include <stddef.h>

int main()
{
    printf("version %s\n", fbgl_version_info());
    printf("name %s\n", fbgl_name_info());

    fbgl_t buffer;
    if (fbgl_init("/dev/fb0", &buffer) == -1) {
        fprintf(stdout, "Error: could not open framebuffer device\n");
        return -1;
    }

    fbgl_set_bg(&buffer, 0xFFFFFF); // Set background color to red

 

    // Load the PSF2 font
    fbgl_psf2_header_t *font = fbgl_load_psf2_font("../asset/font.psf");
    if (!font) {
        fprintf(stderr, "Error: failed to load PSF2 font.\n");
        fbgl_destroy(&buffer);
        return -1;
    }

    printf("Loaded PSF2 Font: %d glyphs, %dx%d px per character\n", font->numglyphs, font->width, font->height);

    // Render sample text
    fbgl_render_text(&buffer, buffer.width, buffer.height, 100, 100, "Hello, framebuffer!", font);

    // Main loop checking for ESC key
    int l = 0;
    while (1) {
        if (fbgl_check_esc_key()) {
            fprintf(stdout, "ESC pressed\n");
            break;
        }
    }

    // Free the font memory
    fbgl_free_psf2_font(font);

    fbgl_destroy(&buffer);
    return 0;
}
