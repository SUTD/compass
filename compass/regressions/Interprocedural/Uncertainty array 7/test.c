// Interprocedural/Uncertainty array 7

#include <stdlib.h>

void test(int* a, int*b, int flag)
{
	int x = a[2];
	int y = b[2];
	assign(a, b,"i>=0 & i < 10 & j=i", "false");
	int p =9999;
}

void foo(int* x, int* y, int k)
{
	int t = 5;
	test(x, y, 1);
	if(x[5] == 2)  t=1;
	if(x[5] == 2) static_assert(t==1);
	int a=3;
	
	
	
}

