// Interprocedural/Call 40

#include <stdlib.h>

int foo(int a, int* x)
{
	if(a+x[1] == 0) 
		return -1;
	else return 1;
}

void bar()
{
	int b = 1;
	int* c = malloc(sizeof(int)*4);
	c[0]= c[1] = -1;
	int r = foo(b, c);
	static_assert(r == -1);
	
}