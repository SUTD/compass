// Interprocedural/Call 3
#include <stdlib.h>
void bar(int *a, int flag)
{
	if(flag) *a = 55;
}

void foo(int flag)
{
	int* b = malloc(sizeof(int));
	*b = 33;
	bar(b, flag);
	if(flag)
		static_assert(*b==55);
	else static_assert(*b == 33);
}
