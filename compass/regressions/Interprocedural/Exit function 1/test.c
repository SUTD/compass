// Interprocedural/Exit function 1

#include <stdlib.h>

void foo(int flag)
{
	if(flag) exit(0);
	
}

void bar(int flag)
{
	int a = 1;
	if(flag) a = 7;
	foo(flag);
	static_assert(a == 1);
	
}
