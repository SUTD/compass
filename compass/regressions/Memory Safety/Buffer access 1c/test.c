// Memory Safety/Buffer access 1c
#include <stdlib.h>


void specify_checks()
{
	check_buffer();
}

void foo()
{
	int a[4];
	// underrun
	int t = a[-1];
}
