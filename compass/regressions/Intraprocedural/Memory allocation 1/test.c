// Intraprocedural/Memory allocation 1
#include <stdlib.h>
void foo(int flag)
{
	int* x = malloc(4);
	*x = 2;
	static_assert(*x==2);
	

}