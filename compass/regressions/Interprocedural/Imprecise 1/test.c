// Interprocedural/Imprecise 1

#include <stdlib.h>

int foo()
{
	int* x = malloc(4);
	if(x==0) return 0;
	return 1;
}

void bar()
{
	int y = foo();
	int z;
	if(y==1)
		z = 3;
	else z = 6;

	if(y==1) static_assert(z==3);

}