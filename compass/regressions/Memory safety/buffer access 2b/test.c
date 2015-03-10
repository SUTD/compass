// Memory safety/buffer access 2b
#include <stdlib.h>


void specify_checks()
{
	check_buffer();
}

void foo()
{
	int a[4];
	a[3] = 2;
}
