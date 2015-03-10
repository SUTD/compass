// Interprocedural/Call 13
#include <stdlib.h>

struct b_s{
	int a;
	int b;
};

struct s{
	int y;
struct b_s b;
};

void bar(struct s* my_s, void* dd_s)
{
	struct s *t = (struct s*)dd_s;
	*t = *my_s; 
	my_s->b.a =2;
	my_s->b.b =2;
	my_s->y = 2;
}


void foo()
{
	struct s foo_s;
	foo_s.y=1;
	foo_s.b.a = 1;
	foo_s.b.b = 1;
	void* v = malloc(sizeof(struct s));
	bar(&foo_s, v);
	struct s* s2 = (struct s*) v;
	static_assert(foo_s.y == 2);
	static_assert(foo_s.b.a == 2);
	static_assert(foo_s.b.b == 2);

	static_assert(s2->y == 1);
	static_assert(s2->b.a == 1);
	static_assert(s2->b.b == 1);

}
