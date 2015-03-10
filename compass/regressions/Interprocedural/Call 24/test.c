//Interprocedural/Call 24
#include <stdlib.h>

struct s {
int x[2];
};

void foo(int* b, struct s* a, int flag)
{
	int* y= a->x;
	if(flag) *b= y[1];
}


void bar()
{
	struct s a;
	int b = 55;
	int flag;
	a.x[1] = 7;
	 foo(&b, &a, flag);
	if(flag) static_assert(b == 7);
	else static_assert(b==55);
	
}