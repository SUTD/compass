// Interprocedural/Call 43
#include <stdlib.h>

void foo(int*** a)
{
	int *b = **a;
	*a = malloc(4);
	*b =77;
}

void bar(int ***x)
{
	int* t = **x;
	foo(x);
	static_assert(*t == 77);
}
