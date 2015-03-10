// Interprocedural/Instantiation 19b
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
	a[1] = 1;
	baz(&a[0], &y); 
	// should fail
	static_assert(y==1);
}
