// Memory Safety/Buffer access 1b
#include <stdlib.h>


void specify_checks()
{
	check_buffer();
}

void foo()
{
	int a[4];
	int t = a[3];
}
