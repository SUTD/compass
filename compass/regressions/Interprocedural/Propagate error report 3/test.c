// Interprocedural/Propagate error report 3

#include <stdlib.h>

void foo(int flag)
{
	static_assert(flag> 88);
}

void bar(int x)
{
	foo(x);
}
void bar2(int x)
{
	bar(x);
}

// This is supposed to fail
void fail(int y)
{
	bar2(y);
}