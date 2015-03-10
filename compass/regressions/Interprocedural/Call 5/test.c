// Interprocedural/Call 5
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
	bar(b, 1);
	if(flag)
		static_assert(*p==55);
	static_assert(*b == 55);
}
