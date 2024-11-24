#define FBGL_IMPLEMENTATION
#include "fbgl.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
	fbgl_t fb;
	fbgl_init("/dev/fb0", &fb);

	fbgl_set_bg(&fb, 0xFFFFFF);

	while (1) {
		if (fbgl_check_esc_key()) {
			printf("pressed");
			fbgl_set_bg(&fb, 0x000000);
		}
	}

	return 0;
}
