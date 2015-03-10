// Interprocedural/Propagate error report 1

#include <stdlib.h>

void foo(int flag)
{
	static_assert(flag == 0);
}

void bar(int x)
{
	foo(x);
}

void success()
{
	bar(0);
}
