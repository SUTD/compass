// Interprocedural/Exit function 3

#include <stdlib.h>

void foo(int** a)
{
	if(a[0] == NULL) exit(0);
	
}

void bar(int** a, int flag)
{
	int b = 1;
	if(a[0] == NULL) b = 7;
	if(flag) foo(a);
	if(flag) static_assert(b == 1);
	
}
