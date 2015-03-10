// Intraprocedural/Cast ptr to int 2

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
	d->r = 4;
	static_assert(a->p.r == 4);

}