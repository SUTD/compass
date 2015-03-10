// Interprocedural/Call 18
#include <stdlib.h>

void foo(int* a)
{
	*a = 1;
}

void bar()
{
	int a[4];
	a[0] = 0;
	foo(a);
	static_assert(a[0] == 1);

}
