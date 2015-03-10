// Memory Safety/Buffer access 1
#include <stdlib.h>


void specify_checks()
{
	check_buffer();
}

void foo()
{
	int a[4];
	// should fail
	int t = a[4];
}
