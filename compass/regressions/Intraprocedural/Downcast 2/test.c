// Intraprocedural/Downcast 2

struct n1{
	int a;
};

struct n2{
	int a;
	int b;
};

struct s1{
	int x;
 	int y;
	struct n1* nested;
};


void foo(struct s1* a)
{
	a->nested->a = 8;
	struct n1* b = a->nested;
	struct n2* c = (struct n2*) b;
	static_assert(c->a == 8);
	
	
}