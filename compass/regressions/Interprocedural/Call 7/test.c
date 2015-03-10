// Interprocedural/Call 7
#include <stdlib.h>

struct s{
	int x;
	int y;
};

void bar(struct s my_s)
{
	my_s.x = 2;
	my_s.y = 3;
}

void foo()
{
	struct s foo_s;
	foo_s.x = 1;
	foo_s.y =1;
	bar(foo_s);
	static_assert(foo_s.x ==1);
	static_assert(foo_s.y == 1);
}
