// Interprocedural/Swap 5
#include <stdlib.h>

/*
a -> b
b->c
c->a
*/
void swap(int* a, int *b, int* c)
{
	int t = *a;
	*a = *b;
	*b = *c;
	*c = t;
}

void foo()
{
	int a = 1;
	int b= 2;
	int c = 3;
	swap(&a, &b, &c);
	swap(&a, &b, &c);
	static_assert(a == 3);	
	static_assert(b==1);
	static_assert(c==2);
}