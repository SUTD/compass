// Interprocedural/Call 42

#include <stdlib.h>

int foo(int a, int* x)
{
	if(a+x[1] == 0) 
		return -1;
	else return 1;
}

void bar(int* c)
{
	int b = 1;
	int r = foo(b, c);
	c++;
	if(c[0] == -1)
		static_assert(r == -1);
	
}