// Intraprocedural/Cast 2

struct s1{
	int x;
 	int y;
};

struct s2 {
	int x;
};


// Downcast
void foo(struct s2* a)
{
	a->x = 4;
	struct s2* c=a;
	struct s1* b = (struct s1*)a;
	static_assert(b->x == 4);
	
}