// Interprocedural/Instantiation 15
#include <stdlib.h>

void  foo(int* x, int* c)
{
	c[1] = 2;
	c[2] = x[5];
}

void bar(int* a, int* b)
{
	a[8] = 77;
	b[0] = 0;
	b[1] =1;
	foo(a+2, b+1);
	static_assert(b[2] == 2);
	static_assert(b[3] == a[7]);

}

void baz()
{
  int a[10];
  int b[10];
  bar(a, b);
}