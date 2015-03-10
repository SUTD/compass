// Memory safety/uninit 2
#include <stdlib.h>


void specify_checks()
{
	check_uninit();
}

void foo()
{
	int x[3];
	x[0]=1;
	int a = x[0];
}
