// Interprocedural/Uncertainty array 22

#include <stdlib.h>

void test(int* a, int*b, int flag)
{
	int y = b[2];
	//if(flag)
	assign(a, *b,"true", "false");
	//else
	//assign(a, 0,"true", "true");
	int p =9999;
}

void foo(int* x, int* y, int k, int f)
{
	int t = *x;
	test(x, y, 1);
	if(*x==4) t = 1;
	if(f==1)
	{
		 test(x, y, 1);
	}
	if(f!=1){
		 if(*x==4) static_assert(t==1);
	 }
	int a=3;
	
	
	
}
