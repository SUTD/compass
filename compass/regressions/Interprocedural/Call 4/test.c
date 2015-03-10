// Interprocedural/Call 4
#include <stdlib.h>
void bar(int *a, int flag)
{
	if(flag) 
		*a = 55;
}

void foo(int flag, int* p)
{
	int* b = malloc(sizeof(int));
	if(flag) b=p;
	bar(b, flag);
	if(flag)
		static_assert(*p==55);
}
