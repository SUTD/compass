// Memory safety/uninit 1
#include <stdlib.h>


void specify_checks()
{
	check_uninit();
}

void foo()
{
	int x;
	int a = x;
}
