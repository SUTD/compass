// Interprocedural/Instantiation 16
#include <stdlib.h>

void  foo(int* x, int* c)
{
	c[1] = 2;
	c[2] = x[5];
	c[3] = x[6];
}

void bar(int* a, int* b)
{
	a[8] = 77;
	b[0] = 0;
	b[1] =1;
	foo(a+2, b+1);
	static_assert(b[2] == 2);
	static_assert(b[3] == a[7]);
	 static_assert(b[4] == 77);
	 static_assert(b[4] == a[8]);
	
}

void baz()
{
  int a[10];
  int b[10];
  bar(a, b);
}