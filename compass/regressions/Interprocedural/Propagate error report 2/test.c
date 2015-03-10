// Interprocedural/Propagate error report 2

#include <stdlib.h>

void foo(int flag)
{
	static_assert(flag == 0);
}

void bar(int x)
{
	foo(x);
}

// This is supposed to fail
void fail()
{
	bar(1);
}
