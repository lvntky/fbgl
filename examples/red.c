#define FBGL_IMPLEMENTATION
#include "fbgl.h"
#include <stdio.h>

int main(void)
{
	fbgl_t fb;
	fbgl_init(NULL, &fb);
	fbgl_set_bg(&fb, 0x00FF0000);
	while (1) {
	}

	return 0;
}
