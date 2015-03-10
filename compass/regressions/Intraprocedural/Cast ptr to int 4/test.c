// Intraprocedural/Cast ptr to int 4

struct e
{
int l;
int r;
int m;
};

struct s
{
int x;
struct e p;
int y;
};


void foo(struct s* a)
 {
	long int b = (long int)a;
	long int c = b+4;
	struct e* d = (struct e*)c;
	long int w = (long int) d;
	long int f = w+8;
	int*g = (int*) f;
	*g=123;
	long int t = f+4;
	int *tt = (int*)t;
	*tt = 555;
	static_assert(a->p.m== 123);
	static_assert(a->y == 555);
	

}