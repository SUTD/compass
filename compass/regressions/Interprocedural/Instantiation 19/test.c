// Interprocedural/Instantiation 19
#include <stdlib.h>

void foo(int x, int* y)
{
	if(x==2) *y=1;
}

void baz(int* p, int* y)
{
	int x = p[1];
	foo(x, y);
	
}

void bar()
{
	int y;
	int a[3];
	a[1] = 2;
	baz(&a[0], &y); 
	static_assert(y==1);
}
