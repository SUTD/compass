// Interprocedural/Call 21
#include <stdlib.h>

void foo(int* a, int num)
{
	*a = num;
}

void bar()
{
	int a[4];
	a[0] = 0;
	int* b = &a[0];
	b++;
	foo(b, 4);
	int* c = ++b;
	foo(c, 6);
	static_assert(a[0] == 0);
	static_assert(a[1] == 4);
	static_assert(a[2] == 6);

}
