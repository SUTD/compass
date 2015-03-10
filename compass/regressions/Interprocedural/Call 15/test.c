// Interprocedural/Call 15
#include <stdlib.h>

void foo(int* a)
{
	a[1]=1;
	a[2] =2;
}

void bar()
{
	int* a = malloc(sizeof(int) * 4);
	foo(a);
	a++;
	static_assert(a[0] == 1);
	static_assert(a[1] == 2);
}
