// Memory safety/uninit 3
#include <stdlib.h>


void specify_checks()
{
	check_uninit();
}

void foo(int* x)
{
	// uninitialized read
	int a = x[1];
}

void bar()
{
	int x[10];
	foo(x);
}
