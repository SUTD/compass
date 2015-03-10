//Interprocedural/Call 24b
#include <stdlib.h>

struct s {
int x[2];
};

void foo(int* b, struct s* a, int flag)
{
	if(flag) *b= a->x[1];
}

void bar()
{
	struct s a;
	int b = 55;
	int flag;
	a.x[1] = 7;
	 foo(&b, &a, flag);
	if(flag) {
		static_assert(b == 8); // should fail
	}
	else static_assert(b==55);
	
}
