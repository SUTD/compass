//Interprocedural/Call 22
#include <stdlib.h>

struct s {
int x[2];
};

void foo(struct s* a)
{
	a->x[2] = 2;
	a->x[1] = 1;
}

void bar()
{
	struct s a;
	foo(&a);
	static_assert(a.x[2] == 2);
	static_assert(a.x[1] == 1);
}
