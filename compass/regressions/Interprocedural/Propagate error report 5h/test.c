// Interprocedural/Propagate error report 5h
#include <stdlib.h>

void foo(int* x)
{
	static_assert( x[1]==2);
}

void bar(int flag, int y, int z)
{
	int a[3]; 
	if(flag)
		a[1]=y;
	else a[1]=z;
	foo(&a[0]);
}

void baz(int flag, int* p)
{
	int y =p[1]; 
	bar(flag, y, 2);
}

void poo(int flag)
{
	int a[3];
	a[1] =2;
	baz(flag, &a[0]); 
}