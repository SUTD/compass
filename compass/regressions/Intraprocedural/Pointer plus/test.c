// Intraprocedural/Pointer plus

struct s {
	int x;
	int y;
};

void foo(struct s a)
{
	a.y =1;
	a.x=2;
	int* b = &(a.y);
	int* c = &a.x;
	int t = a.y;
	*b =a.x;
	*c = t;

	static_assert(a.x == 1);
	static_assert(a.y == 2);

	
}