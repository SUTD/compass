// Interprocedural/Call 31

#include <stdlib.h>

void foo(int** a)
{
	*a[4] = 66;
}

void bar(int* x, int* y)
{
	*x = -1;
	int** array = malloc(sizeof(int*) *20);
	array[4] = x;
	array[5] = y;
	array++;
	foo(array);
	static_assert(*x == -1);
	static_assert(*y==66);
	
	

}