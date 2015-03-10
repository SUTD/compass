// Intraprocedural/Pointer array 2

struct s1{
	int x;
	int y[3];
};

void foo(struct s1 *a)
{
	a[2].x = 3;
	struct s1* b = a+2;
	static_assert(b[0].x==3);
}