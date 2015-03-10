// Interprocedural/Uncertainty array 24

#include <stdlib.h>

void test(int* a, int*b, int flag)
{
	int x = a[2];
	int y = b[2];
	//if(flag)
	assign(a, *b,"i>=0 & i < 10", "false");
	int p =9999;
}

void foo(int* x, int* y, int k)
{
	int t = 5;
	test(x, y, k);
	if(x[4] == 0)  t=1;
	if(x[4] ==0) static_assert(t==1);
	int a=3;
	
	
	
}

void foo2(int* x, int* y, int k)
{
	int t = 5;
	foo(x, y, k);
	if(x[4] == 0)  t=1;
	test(x, y, k);
	//should fail
	if(x[4] ==0) static_assert(t==1);
	int a=3;
	
	
	
}
