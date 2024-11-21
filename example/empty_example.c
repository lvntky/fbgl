#define FBGL_IMPLEMENTATION
#include "../fbgl.h"

#include <stdio.h>

int main()
{
	printf("version %s\n", fbgl_version_info());
	printf("name %s\n", fbgl_name_info());
	fbgl_t buffer;
	if(	fbgl_init("/dev/fb0", &buffer) == -1) {
	    fprintf(stdout, "error could not oppen fb device");
	    return -1;
	}
	fbgl_set_bg(&buffer, 0xFF0000);
	while(1){}

	return 0;
}
