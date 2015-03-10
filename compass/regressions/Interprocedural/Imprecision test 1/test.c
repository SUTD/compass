// Interprocedural/Imprecision test 1

#include <stdlib.h>

int foo()
{
	int* p = malloc(sizeof(int));
	if(p == NULL) return 0;
	return 1;
	
}

int bar()
{
	int a;
	int x = foo();
	if(x) a=3;
	else a=2;
	if(x) static_assert(a==3);
}
