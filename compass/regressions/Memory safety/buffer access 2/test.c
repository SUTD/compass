// Memory safety/buffer access 2
#include <stdlib.h>


void specify_checks()
{
	check_buffer();
}

void foo()
{
	int a[4];
	a[4] = 2;
}
