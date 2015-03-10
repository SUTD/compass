// Intraprocedural/Upcast

struct s1{
	int x;
 	int y;
};

struct s2 {
	int x;
};


// Upcast
void foo(struct s1* a)
{
	a->x = 3;
	struct s1* c=a;
	struct s2* b = (struct s2*)a;
	static_assert(b->x == 3);
	
}