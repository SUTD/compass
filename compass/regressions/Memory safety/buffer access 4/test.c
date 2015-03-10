// Memory safety/buffer access 4
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
	int t;
	b[2] = t;
}
