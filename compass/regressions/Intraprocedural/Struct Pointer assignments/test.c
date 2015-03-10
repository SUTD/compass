// Intraprocedural/Struct Pointer assignments
#define NULL 0

struct s
{
	int f;
	int g;
};

void foo(struct s* a, struct s* b)
{
	a->f= 1;
	a->g = 1;
	b->f=2;
	b->g=2;

	struct s* t = b;	
	b=a;
	a=t;
	
	static_assert(a->f == a->g && a->g == 2);
	static_assert(b->f == 1 && b->g == 1);
	

}