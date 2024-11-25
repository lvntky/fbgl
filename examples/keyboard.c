#define FBGL_IMPLEMENTATION
#include "../fbgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for usleep

int main(int argc, char *argv[])
{
    fbgl_t buf;
    fbgl_init("/dev/fb0", &buf);
    fbgl_set_bg(&buf, 0x000000);

    size_t framesize = 30 * 30;

    fbgl_point_t start = {200, 200};
    fbgl_point_t end = {400, 400};

    fbgl_draw_rectangle_filled(start, end, 0xFFFFFF, &buf);
    fbgl_keyboard_init();

    for(size_t i = 0; i < framesize; i++) {
	usleep(10000);
    }

    fbgl_destroy(&buf);
    
    return 0;
}
