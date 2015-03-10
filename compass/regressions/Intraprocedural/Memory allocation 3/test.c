// Intraprocedural/Memory allocation 3

#include <stdlib.h>

void foo(int** x, int flag)
{
	if(flag) {
		*x = (int*)malloc(4);
	}
	**x = 2;
	static_assert(**x==2);
	

}