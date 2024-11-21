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
		fprintf(stdout, "error could not oppen fb device");
		return -1;
	}
	fbgl_set_bg(&buffer, 0xFF0000);

	for (size_t i = 0; i < 100; ++i) {
		for (size_t j = 0; j < 100; ++j) {
			fbgl_put_pixel(i, j, 0x00FFFF, &buffer);
		}
	}
	int l = 0;
	while (1) {
	    if(fbgl_check_esc_key()) {
		fprintf(stdout, "esc pressed", fbgl_check_esc_key());

	    }
	}

	return 0;
}
