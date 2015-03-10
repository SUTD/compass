// Interprocedural/Pointer arithmetic instantiation 2

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
	// should fail
	a++;
	foo(a, b);
}