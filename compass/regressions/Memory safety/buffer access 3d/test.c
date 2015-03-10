// Memory safety/buffer access 3d
#include <stdlib.h>


void specify_checks()
{
	check_buffer();
}

void foo()
{
	int a[4];
	int* b = &a[0];
	b++;
	int t = b[2];
}
