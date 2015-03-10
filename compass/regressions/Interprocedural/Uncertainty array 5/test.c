// Interprocedural/Uncertainty array 5

#include <stdlib.h>

void test(int* a, int flag)
{
	int x = a[2];
	assign(a,0,"i>=0 & i < 10", "false");
	int p =9999;
}

void foo(int* b, int k)
{
	int y = 5;
	test(b,1);
	if(b[5] == 2)  y=1;
	if(k==5) { 
		if(b[k] == 2) static_assert(y==1);
	}
	
	
}
