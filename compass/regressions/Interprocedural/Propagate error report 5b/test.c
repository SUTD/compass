// Interprocedural/Propagate error report 5b
#include <stdlib.h>

void foo(int x)
{
	static_assert( x==2); //should fail
}

void bar(int flag, int y, int z)
{
	int a = -1;
	if(flag)
		a=y;
	else a=z;
	foo(a);
}

void baz(int flag)
{
	int y, z;
	y=2;
	z=1;
	bar(flag, y, z);
}