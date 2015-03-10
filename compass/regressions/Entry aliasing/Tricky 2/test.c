//Interprocedural/Call 25
#include <stdlib.h>

struct s {
int x[4];
};

void foo(int* a, int* b)
{
	*b = 3;
	*b= a[1];
}

void bar()
{
	int array[4];
	array[1] = 2;
	foo(array, &array[1]);
	static_assert(array[1] == 2); //should fail
}