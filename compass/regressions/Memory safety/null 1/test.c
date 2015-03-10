// Memory safety/null 1
#include <stdlib.h>


void specify_checks()
{
	check_null();
}

void foo(int* x)
{
	int y[5];
	y[1] = 4;
	int a = x[2];
}

void bar()
{
	int x[4];
	x[0] = x[1] = x[2] = 3;
	x[3] = NULL;
	foo(x);
}
