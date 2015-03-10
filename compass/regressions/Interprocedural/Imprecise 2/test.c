// Interprocedural/Imprecise 2

#include <stdlib.h>

int foo()
{
	int* x = malloc(4);
	if(x==0) return 0;
	return 1;
}

int bar()
{
	int y = foo();
	int z;
	if(y==1)
		z = 3;
	else z = 6;

	if(y==1) static_assert(z==3);

	return z==3;

}

int bar2()
{
	int y = bar();
	int z;
	if(y==1)
		z = 3;
	else z = 6;

	if(y==1) static_assert(z==3);

	return z==3;

}