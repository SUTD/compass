//Interprocedural/Call 28
#include <stdlib.h>

struct s {
int x[4];
};

void foo(int* a, int* b)
{
	*b= a[1];
	b[1] = a[2];
}

void bar()
{
	int x[2];
	int array[4];
	array[1] = 2;
	array[2] = 6;
	foo(array, x);
	static_assert(x[0]==2);
	static_assert(x[1]==6);
}