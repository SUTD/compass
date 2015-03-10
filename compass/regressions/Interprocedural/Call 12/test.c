// Interprocedural/Call 12

#include <stdlib.h>

struct s {
	int x;
	int y;
};

void foo(struct s** my_s, int flag)
{
	*my_s = malloc(sizeof(struct s));
	if(flag) {
		(*my_s)->x =1;
		(*my_s)->y= 1;
	}
	else {
		(*my_s)->x =2;
		(*my_s)->y= 2;
	}
}

void bar(int flag)
{
	struct s* ss;
	foo(&ss, flag);
	if(flag) static_assert(ss->x == 1);
	else static_assert(ss->x == 2);
	assert(ss->x == ss->y);
}
