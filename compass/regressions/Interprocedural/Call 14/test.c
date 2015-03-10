// Interprocedural/Call 14
#include <stdlib.h>

void foo(int* a)
{
	a[1]=1;
	a[2] =2;
}

void bar()
{
	int b[3] ; // so we get index var naming collisions
	b[1] =0; 
	int* a = malloc(sizeof(int) * 4);
	foo(a);
	static_assert(a[1] == 1);
	static_assert(a[2] == 2);
}
