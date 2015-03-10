// Interprocedural/Uncertainty array 2

#include <stdlib.h>

void test(int* a, int flag)
{
	int x = a[2];
	assign(a,0,"i>=0 & i < 10", "false");
	int p =9999;
}

void foo(int* b)
{
	int y = 5;
	test(b,1);
	if(b[5] == 2)  y=1;
	//should fail
	if(b[5] == 3) static_assert(y==1);
	
	int t = 7;
	
}

