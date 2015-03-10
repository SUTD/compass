// Intraprocedural/Get field 2

struct bar{
	int count;
	char* id;
};

struct s{
	int a;
	char b;
	struct bar c;
};

void foo(struct s s1, char p)
{
	s1.c.count = 999;
	s1.c.id = &p;
	struct bar cc = s1.c;
	int v1 = cc.count;
	char* v2 = cc.id;
	int x = s1.c.count;
	static_assert(*v2 == p);
	static_assert(x == 999);
}