#define FBGL_IMPLEMENTATION
//#define FBGL_HIDE_CURSOR
#define FBGL_USE_FREETYPE
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

    FT_Library library = fbgl_freetype_init();
    if (!library) {
        fbgl_destroy(&buffer);
        return -1;
    }

    FT_Face face = fbgl_load_font(library, "../asset/font_2.ttf", 24);  // Adjust path and size
    if (!face) {
        fbgl_freetype_cleanup(library);
        fbgl_destroy(&buffer);
        return -1;
    }

    // Render text to framebuffer
    fbgl_render_freetype_text(&buffer, library, face, "Hello, World!", 50, 50);

    // Main loop checking for ESC key
    int l = 0;
    while (1) {
        if (fbgl_check_esc_key()) {
            fprintf(stdout, "ESC pressed\n");
            break;
        }
    }
    fbgl_destroy(&buffer);
    return 0;
}
