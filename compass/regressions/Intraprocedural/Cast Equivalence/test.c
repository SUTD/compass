// Intraprocedural/Cast Equivalence

struct s1
{
	int x;
	int y;
};

struct s2
{
	int a;
	int b;
};

void foo(void* a)
{
	struct s1* s = (struct s1*)a;
	int x = s->x;
	struct s2* ss = (struct s2*)a;
	int y = ss->a;
	
	int z = 0;
	if(x==y) z = 1;
	static_assert(z==1);
}



