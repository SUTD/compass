// Interprocedural/Propagate error report 5a
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

void baz(int flag)
{
	int y, z;
	y=z=2;
	bar(flag, y, z);
}