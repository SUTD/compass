// Interprocedural/Call 23
#include <stdlib.h>

struct s {
int* x;
};

void foo(struct s* a)
{
	a->x[2] = 2;
	a->x[1] = 1;
}

void bar()
{
	int b[2];
	b[0]=0;

	struct s a;
	*a.x = 2;
	a.x[1] = 3;

	foo(&a);
	static_assert(a.x[0] == 2);
	static_assert(a.x[1] == 1);
	static_assert(a.x[2] == 2);
	
}
