//Interprocedural/Call 26
#include <stdlib.h>

struct s {
int x[4];
};

void foo(int* a, int* b)
{
	*b= a[1];
}

void bar()
{
	int x;
	int* array = malloc(sizeof(int));
	array[1] = 2;
	foo(array, &x);
	static_assert(x==2);
}