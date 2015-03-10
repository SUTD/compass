// Interprocedural/Instantiation 22

#include <stdlib.h>

int* foo()
{
	int* t= malloc(sizeof(int)*10);
	t[1] = 1;
	return t;
	
}

void bar()
{
	int* res = foo();
	int a = res[1];
	static_assert(a == 1);
	
}
