// Intraprocedural/Load 2

struct s1 {
	char* id;
	int count; 
};

struct s2{
	int* x;
	struct s1* nested_s;
	int*y;
};

void foo(struct s2* outer)
{
	outer->nested_s->count = 50;
	struct s1* local = outer->nested_s;
	int c = outer->nested_s->count;
	static_assert(c==50);
}