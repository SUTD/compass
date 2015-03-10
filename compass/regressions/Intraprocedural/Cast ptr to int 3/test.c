// Intraprocedural/Cast ptr to int 3

struct e
{
int l;
int r;
int m;
};

struct s
{
struct e p;
int x;
int y;
};


void foo(struct s* a) 
{
	long int b = (long int)a;
	long int c = b+0;
	struct e* d = (struct e*)c;
	long int w = (long int) d;
	long int f = w+8;
	int*g = (int*) f;
	*g=123;
	static_assert(a->p.m == 123);
	

}