#define FBGL_IMPLEMENTATION
#include "fbgl.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
	fbgl_t fb;
	fbgl_init("/dev/fb0", &fb);
	fbgl_set_bg(&fb, 0x00FF0000);
	while (1) {
	}

	return 0;
}
