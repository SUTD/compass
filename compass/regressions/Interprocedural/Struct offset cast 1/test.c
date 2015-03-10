// Interprocedural/Struct offset cast 1

struct s
{
	int x;
	int y;
};

struct s2
{
	int a;
	int b;
struct s my_s;
};




void* foo(void* a)
{
	struct s* t = (struct s*)a;
	t->x = 1;
	t->y = 2;
	return t;
}

void bar(void* b)
{
	struct s2* dd = (struct s2*)b;
	dd->a = 99;
	dd->b = 55;
	struct s* e = &dd->my_s;
	
	void* c = foo((void*) e);
	
	struct s2* z = (struct s2*)dd;
	static_assert(z->a == 99);
	static_assert(z->b == 55);
	static_assert(z->my_s.x == 1);
	static_assert(z->my_s.y == 2);
	
	struct s* cc = (struct s*) c;
	static_assert(cc->x == 1);
	
}
