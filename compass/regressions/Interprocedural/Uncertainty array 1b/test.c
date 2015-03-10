// Interprocedural/Uncertainty array 1b

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
	if(b[5] == 2){
	int t = (y==1);
        static_assert(t);
	 }
	
	int t = 7;
	
}
