// Intraprocedural/Cast ptr to int

struct s
{
int x;
int y;
};


void foo(struct s* a)
{
	a->x = 1;
	a->y = 2;
	long int b = (long int)a;
	long int c = b+4;
	int* d = (int*)c;
	int* e = (int*) b;
	static_assert(*d == 2);
	static_assert(*e == 1);
}