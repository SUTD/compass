// Interprocedural/Instantiation 10
#include <stdlib.h>

void foo(int* a)
{
	a[1] = 1;
	a[2] = 2;
	a[3]= a[2];
}

void bar()
{
	int b[3] ; // so we get index var naming collisions
	b[1] =0; 
	int* a = malloc(sizeof(int) * 4);
	a[1] = 0;
	foo(a);
	static_assert(a[1] == 1);
	static_assert(a[2] == 2);
	static_assert(a[3] == 2);
}