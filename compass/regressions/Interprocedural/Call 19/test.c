//Interprocedural/Call 19
#include <stdlib.h>

struct s {
int x[2];
};

void foo(struct s* a)
{
	a->x[1] = 1;
}

void bar()
{
	int b[4];
	b[1] = 2;


	struct s a[4];
	foo(&a[0]);
	static_assert(a[0].x[1] == 1);

	foo(&a[1]);
	static_assert(a[1].x[1] == 1); 
	
}