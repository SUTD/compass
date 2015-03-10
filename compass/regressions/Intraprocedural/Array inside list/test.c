// Intraprocedural/Array inside list

struct s;
struct s {
	int hd;
	int x[3];
	struct s* tl;
};

void foo(struct s* a)
{
	int* y = &a->tl->x[0];
	*y = 4;
	static_assert(a->tl->x[0] == 4);
	++y;
	*y = 5;
	static_assert(a->tl->x[1] == 5);
	
	int* z = &a->tl->tl->x[1];
	z[1] = 6;
	static_assert(a->tl->tl->x[2] == 6);
	
	
}