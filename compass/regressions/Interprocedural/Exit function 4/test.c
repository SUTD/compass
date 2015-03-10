// Interprocedural/Exit function 4

#include <stdlib.h>

void foo(int* a)
{
	static_assert(a!=NULL);
	
}

void bar(int* a, int flag)
{
	int* p =malloc(4);
	if(p == NULL) exit(1);
	foo(p);
	
}
