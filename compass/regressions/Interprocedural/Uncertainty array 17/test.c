// Interprocedural/Uncertainty array 17

#include <stdlib.h>

void test(int* a, int*b, int flag)
{
	int y = b[2];
	assign(a, *b,"0<=j & j < 5", "false");
	int p =9999;
}

void foo(int* x, int* y, int k)
{
	int t = *x;
	test(x, y, 1);
	if(*x==2) t = 1;
	// should fail
	if(*x==3) static_assert(t==1);
	int a=3;
	
	
	
}
