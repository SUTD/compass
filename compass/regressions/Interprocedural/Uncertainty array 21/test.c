// Interprocedural/Uncertainty array 21

#include <stdlib.h>

void test(int* a, int*b, int flag)
{
	int y = b[2];
	if(flag)
	assign(a, *b,"0<=j & j < 5", "false");
	else
	assign(a, 0,"true", "true");
	int p =9999;
}

void foo(int* x, int* y, int k)
{
	int t = *x;
	test(x, y, 1);
	if(*x==4) t = 1;
	test(x, y, 1);
	//should fail
	if(*x==4) static_assert(t==1);
	int a=3;
	
	
	
}
