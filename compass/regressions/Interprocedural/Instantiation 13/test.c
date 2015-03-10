// Interprocedural/Instantiation 13
#include <stdlib.h>

void foo(int* a, int* c)
{
	a[1] = 1;
	a[2] = 2;
	a[3]= c[2];
}

void bar()
{
	int b[10] ;
	b[1] =0; 
	b[6] = 66;
	int* a = malloc(sizeof(int) * 10);
	a[1] = 0;
	foo(a+1, b+4);
	static_assert(a[2] == 1);
	static_assert(a[3] == 2);
	static_assert(a[4] == 66);
}
