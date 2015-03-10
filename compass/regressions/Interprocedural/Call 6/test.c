// Interprocedural/Call 6
#include <stdlib.h>

struct s{
	int x;
	int y;
};

void bar(struct s* my_s)
{
	my_s->x = 2;
	my_s->y = 3;
}

void foo()
{
	struct s foo_s;
	bar(&foo_s);
	static_assert(foo_s.x ==2);
	static_assert(foo_s.y == 3);
}
