// Intraprocedural/Cast ptr to int 5

struct e
{
int l;
int r;
int m;
};

struct s
{
int x;
int y;
struct e p;
};


void foo(struct s* a) 
{
	long int b = (long int)a;
	long int c = b+8;
	struct e* d = (struct e*)c;
	long int w = (long int) d;
	long int f = w+8;
	int*g = (int*) f;
	*g=123;
	static_assert(a->p.m == 123);
}
