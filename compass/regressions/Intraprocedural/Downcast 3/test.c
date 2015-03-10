// Intraprocedural/Downcast 3

struct n1 {
	int a;
 	int b;
};

struct s1{
	int x;
 	int y;
	struct n1 nested;
};


struct s2{
	int x;
};



void foo(struct s2* a)
{
	a->x = 9;
	struct s1* b;
	struct s2* c = a;
	b = (struct s1*) a;
	static_assert(b->x == 9);
}