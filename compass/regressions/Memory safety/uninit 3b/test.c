// Memory safety/uninit 3b
#include <stdlib.h>


void specify_checks()
{
	check_uninit();
}

void foo(int* x)
{
	// correct
	int a = x[1];
}

void bar()
{
	int x[4] = {1, 2, 3, 5};
	foo(x);
}
