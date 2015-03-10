// Interprocedural/Pointer arithmetic instantiation 1

#include <stdlib.h>

void foo(int* a, int* b)
{
	a++;
	b++;
	static_assert(*a == *b);
	
}

void bar(int* a, int* b)
{
	a[1] = b[1];
	foo(a, b);
}