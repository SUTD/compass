// Interprocedural/Call 20
#include <stdlib.h>

void foo(int* a)
{
	*a = 1;
}

void bar()
{
	int a[4];
	a[0] = 0;
	int* b = &a[0];
	b++;
	foo(b);
	static_assert(a[0] == 0);
	static_assert(a[1] == 1);

}
