#define FBGL_IMPLEMENTATION
#include "../include/fbgl/fbgl.h"

#include <stdio.h>

int main()
{
	printf("version %s\n", fbgl_version_info());
	printf("name %s\n", fbgl_name_info());
	fbgl_t buffer;
	fbgl_init("/dev/fb0", &buffer);

	return 0;
}
