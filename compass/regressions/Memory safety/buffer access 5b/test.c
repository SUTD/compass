//Memory safety/buffer access 5b
#include <stdlib.h>


void specify_checks()
{
	check_null();
	check_buffer();
}

void foo(int flag, int b)
{
	int a[10];
	a[b] =3;
	
}

void bar(int* y)
{
	// unsafe buffer access
	foo(0, 10);
}
