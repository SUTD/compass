// Intraprocedural/Store 1
struct s1 {
	char* id;
	int count; 
};

struct s2{
	int* x;
	struct s1 nested_s;
	int*y;
};

void foo(struct s2* outer)
{
	struct s1 t;
	t.count = 99;
	outer->nested_s = t;
	static_assert(outer->nested_s.count == 99);
}