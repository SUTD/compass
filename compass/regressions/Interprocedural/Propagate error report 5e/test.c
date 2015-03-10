// Interprocedural/Propagate error report 5e
#include <stdlib.h>

void foo(int x)
{
	static_assert( x==2);
}

void bar(int flag, int y, int z)
{
	int a = -1;
	if(flag)
		a=y;
	else a=z;
	foo(a);
}

void baz(int flag, int* p)
{
	int y =*p; 
	bar(flag, y, 2);
}

void poo(int flag)
{
	int a =2;
	baz(flag, &a); 
}