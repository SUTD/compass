// Interprocedural/Propagate error report 5d
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
// should fail
void baz()
{
	int y, z;
	y=2;
	z=1;
	bar(0, y, z);
}