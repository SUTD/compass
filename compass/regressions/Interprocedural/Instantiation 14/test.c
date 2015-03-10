// Interprocedural/Instantiation 14
#include <stdlib.h>

int  foo(int* x, int* c, int **z)
{
	
	if(x[4]==0) return 1;
		return -1;
}

void bar()
{
	int*a;
	int*b;
	int** w;
	int res = foo(a+1, b+4, w+2);

}
