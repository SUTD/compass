// Loop/Termination Cond 1

#include <stdlib.h>

void foo(int size, int* a, int* b)
{
	while(1)
	{
		if(a==NULL) break;
	}
	

}

void boo(int size, int*a, int*b)
{
	foo(size, a, b);
	
}	

void bar(int size, int*a, int*b)
{
	boo(size, a, b);
	static_assert(a==0);
}
