// Intraprocedural/Store 2

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
	struct s1 t;
	outer->nested_s->count = 3;
	t = *outer->nested_s;
	static_assert(t.count == 3);

	struct s1* t2;
	t2 = outer->nested_s;
	outer->nested_s->count = 4;
	static_assert(t2->count == 4);
	static_assert(t.count == 3);
}